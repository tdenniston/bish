#include <cassert>
#include "CodeGen_Bash.h"

using namespace Bish;

void CodeGen_Bash::indent() {
    for (unsigned i = 0; i < indent_level; i++) {
        stream << "    ";
    }
}

void CodeGen_Bash::visit(Module *n) {
    for (std::vector<Function *>::const_iterator I = n->functions.begin(),
             E = n->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    // Insert a call to bish_main().
    assert(n->main);
    FunctionCall *call_main = new FunctionCall(n->main);
    visit(call_main);
    stream << ";\n";
    delete call_main;
}

void CodeGen_Bash::visit(Block *n) {
    if (should_print_block_braces()) stream << "{\n";
    indent_level++;
    for (std::vector<IRNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        indent();
        (*I)->accept(this);
        stream << ";\n";
    }
    indent_level--;
    if (should_print_block_braces()) stream << "}\n\n";
}

void CodeGen_Bash::visit(Variable *n) {
    if (should_quote_variable()) stream << "\"";
    stream << "$" << lookup_name(n);
    if (should_quote_variable()) stream << "\"";
}

void CodeGen_Bash::visit(ReturnStatement *n) {
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
    enable_functioncall_wrap();
    n->pblock->condition->accept(this);
    if (is_equals_op(n->pblock->condition) ||
        !dynamic_cast<BinOp*>(n->pblock->condition)) {
        stream << " -eq 1";
    }
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
    stream << "function bish_" << n->name << " ";
    stream << "() ";
    LetScope *s = new LetScope();
    push_let_scope(s);
    // Bash doesn't allow named arguments to functions.
    // We have to translate to positional arguments.
    unsigned i = 1;
    for (std::vector<Variable *>::const_iterator I = n->args.begin(), E = n->args.end(); I != E; ++I, ++i) {
      s->add(*I, convert_string(i));
    }
    if (n->body) n->body->accept(this);
    pop_let_scope();
}

void CodeGen_Bash::visit(FunctionCall *n) {
    const int nargs = n->args.size();
    if (should_functioncall_wrap()) stream << "$(";
    stream << "bish_" << n->function->name;
    for (int i = 0; i < nargs; i++) {
        stream << " ";
        enable_functioncall_wrap();
        if (const FunctionCall *FC = dynamic_cast<const FunctionCall*>(n->args[i])) {
          if (should_quote_variable()) stream << "\"";
          n->args[i]->accept(this);
          if (should_quote_variable()) stream << "\"";
        } else {
            n->args[i]->accept(this);
        }
        reset_functioncall_wrap();
    }
    if (should_functioncall_wrap()) stream << ")";
}

void CodeGen_Bash::visit(ExternCall *n) {
    if (should_functioncall_wrap()) stream << "$(";
    disable_quote_variable();
    for (InterpolatedString::const_iterator I = n->body->begin(), E = n->body->end();
         I != E; ++I) {
        if ((*I).is_str()) {
            stream << (*I).str();
        } else {
            assert((*I).is_var());
            visit((*I).var());
        }
    }
    reset_quote_variable();
    if (should_functioncall_wrap()) stream << ")";
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
    stream << lookup_name(n->variable) << "=";
    enable_functioncall_wrap();
    n->value->accept(this);
    reset_functioncall_wrap();
}

void CodeGen_Bash::visit(BinOp *n) {
    std::string bash_op;
    bool comparison = false, equals = false, string = false;
    if (n->a->type() == StringTy || n->b->type() == StringTy) {
        string = true;
    }
    switch (n->op) {
    case BinOp::Eq:
        bash_op = string ? "==" : "-eq";
        equals = true;
        comparison = true;
        break;
    case BinOp::NotEq:
        bash_op = string ? "!=" : "-ne";
        equals = true;
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

    if (equals && should_comparison_wrap()) stream << "$([[ ";
    if (!comparison) stream << "$((";
    if (!string) disable_quote_variable();
    n->a->accept(this);
    stream << " " << bash_op << " ";
    n->b->accept(this);
    if (equals && should_comparison_wrap()) stream << " ]] && echo 1 || echo 0)";
    if (!comparison) stream << "))";
    if (!string) reset_quote_variable();

    if (reset_wrap && (n->op == BinOp::And || n->op == BinOp::Or)) {
        reset_comparison_wrap();
        stream << " ]] && echo 1 || echo 0)";
    }
}

void CodeGen_Bash::visit(UnaryOp *n) {
    switch (n->op) {
    case UnaryOp::Negate:
        stream << "-";
        break;
    case UnaryOp::Not:
        stream << "$([[ ! ( ";
        disable_comparison_wrap();
        break;
    }
    n->a->accept(this);
    if (n->op == UnaryOp::Not) {
        stream << " ) ]] && echo 1 || echo 0)";
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
    stream << "\"" << n->value << "\"";
}

void CodeGen_Bash::visit(Boolean *n) {
    stream << n->value;
}
