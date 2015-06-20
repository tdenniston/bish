#include <cassert>
#include "CodeGen_Bash.h"

using namespace Bish;

void CodeGen_Bash::indent() {
    for (unsigned i = 0; i < indent_level; i++) {
        stream << "    ";
    }
}

// Return true if the given node is a statement that should be
// emitted. This excludes side-effecting statements like 'import'.
bool CodeGen_Bash::should_emit_statement(const IRNode *node) const {
    return dynamic_cast<const ImportStatement*>(node) == NULL;
}

void CodeGen_Bash::visit(Module *n) {
    // Define the functions first.
    for (std::vector<Function *>::const_iterator I = n->functions.begin(),
             E = n->functions.end(); I != E; ++I) {
	if (!(compile_as_library && (*I)->name.name == "main"))
            (*I)->accept(this);
    }
    // Special case for command-line arguments. TODO: tie this into Builtins somehow.
    stream << "args=( $0 \"$@\" );\n";
    // Global variables next.
    for (std::vector<Assignment *>::const_iterator I = n->global_variables.begin(),
             E = n->global_variables.end(); I != E; ++I) {
        (*I)->accept(this);
        stream << ";\n";
    }
    if (!compile_as_library) {
        // Insert a call to bish_main().
        assert(n->main);
        FunctionCall *call_main = new FunctionCall(n->main);
        visit(call_main);
        stream << ";\n";
        delete call_main;
    }
}

void CodeGen_Bash::visit(Block *n) {
    if (should_print_block_braces()) stream << "{\n";
    indent_level++;

    if (!function_args_insert.empty()) {
        Function *f = function_args_insert.top();
        function_args_insert.pop();
        unsigned i = 1;
        for (std::vector<Variable *>::const_iterator I = f->args.begin(), E = f->args.end(); I != E; ++I) {
            indent();
            stream << "local " << (*I)->name.str() << "=";
            if ((*I)->is_reference()) {
                bool array = (*I)->type().array();
                if (array) stream << "( ";
                (*I)->reference->accept(this);
                if (array) stream << " )";
                stream << ";\n";
            } else {
                stream << "\"$" << i++ << "\";\n";
            }
        }
    }

    for (std::vector<IRNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        if (should_emit_statement(*I)) {
            indent();
            (*I)->accept(this);
	    if (!dynamic_cast<Block *>(*I)) {
		stream << ";\n";
	    }
        }
    }
    // Bash doesn't allow empty functions: must insert a call to a null command.
    if (n->nodes.empty()) {
        indent();
        stream << ": # Empty function\n";
    }
    indent_level--;
    if (should_print_block_braces()) {
	indent();
	stream << "}\n";
    }
}

void CodeGen_Bash::visit(Variable *n) {
    if (should_quote_variable()) stream << "\"";
    bool array = n->type().array();
    stream << "$";
    if (array) stream << "{";
    stream << lookup_name(n);
    if (array) stream << "[@]}";
    if (should_quote_variable()) stream << "\"";
}

void CodeGen_Bash::visit(Location *n) {
    if (should_quote_variable()) stream << "\"";
    if (n->is_variable()) {
        bool array = n->variable->type().array();
        stream << "$";
        if (array) stream << "{";
        stream << lookup_name(n->variable);
        if (array) stream << "[@]}";
    } else {
        assert(n->is_array_ref());
        stream << "${" << lookup_name(n->variable) << "[";
        n->offset->accept(this);
        stream << "]}";
    }
    if (should_quote_variable()) stream << "\"";
}

void CodeGen_Bash::visit(ReturnStatement *n) {
    if (n->value == NULL) {
	stream << "return";
	return;
    }
    bool external = dynamic_cast<ExternCall*>(n->value) != NULL;
    stream << "echo ";
    enable_functioncall_wrap();
    // Defensively wrap external calls in quotes in case they return
    // space-separated strings. Not sure how to handle this yet in the
    // general case.
    if (external) stream << "\"";
    n->value->accept(this);
    if (external) stream << "\"";
    reset_functioncall_wrap();
    stream << "; exit";
}

void CodeGen_Bash::visit(LoopControlStatement *n) {
    switch (n->op) {
    case LoopControlStatement::Break:
        stream << "break";
        break;
    case LoopControlStatement::Continue:
        stream << "continue";
        break;
    }
}

void CodeGen_Bash::visit(IfStatement *n) {
    stream << "if [[ ";
    // Disable comparison wrap because this is within [[ ... ]]
    disable_comparison_wrap();
    enable_functioncall_wrap();
    n->pblock->condition->accept(this);
    if (!dynamic_cast<BinOp*>(n->pblock->condition)) {
        stream << " -eq 1";
    }
    enable_comparison_wrap();
    reset_functioncall_wrap();
    stream << " ]]; then\n";
    disable_block_braces();
    n->pblock->body->accept(this);

    for (std::vector<PredicatedBlock *>::const_iterator I = n->elses.begin(),
             E = n->elses.end(); I != E; ++I) {
        indent();
        stream << "elif [[ ";
        enable_functioncall_wrap();
        (*I)->condition->accept(this);
        reset_functioncall_wrap();
        stream << " ]]; then\n";
        (*I)->body->accept(this);
    }
    if (n->elseblock) {
        indent();
        stream << "else\n";
        n->elseblock->accept(this);
    }

    reset_block_braces();
    indent();
    stream << "fi";
}

void CodeGen_Bash::visit(ForLoop *n) {
    stream << "for " << lookup_name(n->variable) << " in ";
    if (n->upper) {
        stream << "$(seq ";
        n->lower->accept(this);
        stream << " ";
        n->upper->accept(this);
        stream << ")";
    } else {
        disable_quote_variable();
        n->lower->accept(this);
        reset_quote_variable();
    }
    stream << "; do\n";
    disable_block_braces();
    n->body->accept(this);
    reset_block_braces();
    indent();
    stream << "done";
}

void CodeGen_Bash::visit(Function *n) {
    if (n->body == NULL) return;
    stream << "\nfunction " << function_name(n) << " ";
    stream << "() ";
    push_function_args_insert(n);
    if (n->body) n->body->accept(this);
}

void CodeGen_Bash::visit(FunctionCall *n) {
    const int nargs = n->args.size();
    if (should_functioncall_wrap()) stream << "$(";
    stream << function_name(n->function);
    for (int i = 0; i < nargs; i++) {
        // Variables passed by reference are communicated by a global
        // variable, not a function argument.
        if (n->function->args[i]->is_reference()) continue;
        Variable *arg = n->args[i]->location->variable;
        assert(arg);
        stream << " ";
        arg->accept(this);
    }
    if (should_functioncall_wrap()) stream << ")";
}

void CodeGen_Bash::visit(ExternCall *n) {
    if (should_functioncall_wrap()) stream << "$(";
    disable_quote_variable();
    output_interpolated_string(n->body);
    reset_quote_variable();
    if (should_functioncall_wrap()) stream << ")";
}

void CodeGen_Bash::output_interpolated_string(InterpolatedString *n) {
    for (InterpolatedString::const_iterator I = n->begin(), E = n->end();
         I != E; ++I) {
        if ((*I).is_str()) {
            stream << (*I).str();
        } else {
            assert((*I).is_var());
            visit((*I).var());
        }
    }
}

void CodeGen_Bash::visit(IORedirection *n) {
    std::string bash_op;
    switch (n->op) {
    case IORedirection::Pipe:
        bash_op = "|";
        break;
    default:
        assert(false && "Unimplemented redirection.");
    }

    disable_functioncall_wrap();
    stream << "$(";
    n->a->accept(this);
    stream << " " << bash_op << " ";
    n->b->accept(this);
    stream << ")";
    reset_functioncall_wrap();
}

void CodeGen_Bash::visit(Assignment *n) {
    Location *loc = n->location;
    if (!loc->variable->global) stream << "local ";
    if (loc->is_variable()) {
        stream << lookup_name(loc->variable) << "=";
    } else {
        assert(loc->is_array_ref());
        stream << lookup_name(loc->variable) << "[";
        loc->offset->accept(this);
        stream << "]=";
    }
    enable_functioncall_wrap();
    const int nvals = n->values.size();
    assert(nvals > 0);
    bool array = nvals > 1 || n->values[0]->type().array();
    if (array) stream << "( ";
    for (int i = 0; i < nvals; i++) {
        n->values[i]->accept(this);
        if (i < nvals - 1) stream << " ";
    }
    if (array) stream << " )";
    reset_functioncall_wrap();
}

void CodeGen_Bash::visit(BinOp *n) {
    std::string bash_op;
    bool comparison = false, string = false;
    string = n->a->type().string() || n->b->type().string();
    switch (n->op) {
    case BinOp::Eq:
        bash_op = string ? "==" : "-eq";
        comparison = true;
        break;
    case BinOp::NotEq:
        bash_op = string ? "!=" : "-ne";
        comparison = true;
        break;
    case BinOp::LT:
        bash_op = string ? "<" : "-lt";
        comparison = true;
        break;
    case BinOp::LTE:
        bash_op = "-le";
        comparison = true;
        break;
    case BinOp::GT:
        bash_op = string ? ">" : "-gt";
        comparison = true;
        break;
    case BinOp::GTE:
        bash_op = "-ge";
        comparison = true;
        break;
    case BinOp::And:
        bash_op = "&&";
        comparison = true;
        break;
    case BinOp::Or:
        bash_op = "||";
        comparison = true;
        break;
    case BinOp::Add:
        bash_op = "+";
        break;
    case BinOp::Sub:
        bash_op = "-";
        break;
    case BinOp::Mul:
        bash_op = "*";
        break;
    case BinOp::Div:
        bash_op = "/";
        break;
    case BinOp::Mod:
        bash_op = "%";
        break;
    }

    bool reset_wrap = false;
    if (should_comparison_wrap() && (n->op == BinOp::And || n->op == BinOp::Or)) {
        reset_wrap = true;
        // Disable wrapping comparisons in [[]] because we need to
        // wrap them all from the outside for And and Or operations.
        disable_comparison_wrap();
        stream << "$([[ ";
    }

    if (comparison && should_comparison_wrap()) stream << "$([[ ";
    if (!comparison) stream << "$((";
    if (!string) disable_quote_variable();
    n->a->accept(this);
    stream << " " << bash_op << " ";
    n->b->accept(this);
    if (comparison && should_comparison_wrap()) stream << " ]] && echo 1 || echo 0)";
    if (!comparison) stream << "))";
    if (!string) reset_quote_variable();

    if (reset_wrap && (n->op == BinOp::And || n->op == BinOp::Or)) {
        reset_comparison_wrap();
        stream << " ]] && echo 1 || echo 0)";
    }
}

void CodeGen_Bash::visit(UnaryOp *n) {
    bool negate_binop = dynamic_cast<BinOp*>(n->a);
    switch (n->op) {
    case UnaryOp::Negate:
        stream << "-";
        break;
    case UnaryOp::Not:
        stream << "$(! [[ ";
        disable_comparison_wrap();
        break;
    }
    n->a->accept(this);
    if (n->op == UnaryOp::Not) {
        // Don't need the '-eq 1' if the argument is a binary operator (like '==').
        if (!negate_binop) stream << " -eq 1";
        stream << " ]] && echo 1 || echo 0)";
        reset_comparison_wrap();
    }
}

void CodeGen_Bash::visit(Integer *n) {
    stream << n->value;
}

void CodeGen_Bash::visit(Fractional *n) {
    stream << n->value;
}

void CodeGen_Bash::visit(String *n) {
    stream << "\"";
    disable_quote_variable();
    output_interpolated_string(n->value);
    reset_quote_variable();
    stream << "\"";
}

void CodeGen_Bash::visit(Boolean *n) {
    stream << n->value;
}
