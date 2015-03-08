#include <cassert>
#include "CodeGen_Bash.h"

using namespace Bish;

void CodeGen_Bash::indent() {
    for (unsigned i = 0; i < indent_level; i++) {
        stream << "    ";
    }
}

void CodeGen_Bash::visit(const Module *n) {
    for (std::vector<Function *>::const_iterator I = n->functions.begin(),
             E = n->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    // Insert a call to bish_main().
    assert(n->main);
    FunctionCall *call_main = new FunctionCall(n->main->name);
    visit(call_main);
    stream << ";\n";
    delete call_main;
}

void CodeGen_Bash::visit(const Block *n) {
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

void CodeGen_Bash::visit(const Variable *n) {
    if (should_quote_variable()) stream << "\"";
    stream << "$" << lookup_name(n);
    if (should_quote_variable()) stream << "\"";
}

void CodeGen_Bash::visit(const ReturnStatement *n) {
    stream << "echo ";
    enable_functioncall_wrap();
    n->value->accept(this);
    disable_functioncall_wrap();
    stream << "; exit";
}

void CodeGen_Bash::visit(const IfStatement *n) {
    stream << "if [[ ";
    enable_functioncall_wrap();
    n->pblock->condition->accept(this);
    disable_functioncall_wrap();
    stream << " ]]; then\n";
    disable_block_braces();
    n->pblock->body->accept(this);

    for (std::vector<PredicatedBlock *>::const_iterator I = n->elses.begin(),
             E = n->elses.end(); I != E; ++I) {
        indent();
        stream << "elif [[ ";
        enable_functioncall_wrap();
        (*I)->condition->accept(this);
        disable_functioncall_wrap();
        stream << " ]]; then\n";
        (*I)->body->accept(this);
    }
    if (n->elseblock) {
        indent();
        stream << "else\n";
        n->elseblock->accept(this);
    }
    
    enable_block_braces();
    indent();
    stream << "fi";
}

void CodeGen_Bash::visit(const ForLoop *n) {
    stream << "for " << lookup_name(n->variable) << " in ";
    if (n->upper) {
        stream << "$(seq ";
        n->lower->accept(this);
        stream << " ";
        n->upper->accept(this);
        stream << ")";
    } else {
        n->lower->accept(this);
    }
    stream << "; do\n";
    disable_block_braces();
    n->body->accept(this);
    enable_block_braces();
    indent();
    stream << "done";
}

void CodeGen_Bash::visit(const Function *n) {
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
    n->body->accept(this);
    pop_let_scope();
}

void CodeGen_Bash::visit(const FunctionCall *n) {
    const int nargs = n->args.size();
    if (should_functioncall_wrap()) stream << "$(";
    stream << "bish_" << n->name;
    for (int i = 0; i < nargs; i++) {
        stream << " ";
        bool old = enable_functioncall_wrap();
        if (const FunctionCall *FC = dynamic_cast<const FunctionCall*>(n->args[i])) {
          if (should_quote_variable()) stream << "\"";
          n->args[i]->accept(this);
          if (should_quote_variable()) stream << "\"";
        } else {
          n->args[i]->accept(this);
        }
        set_functioncall_wrap(old);
    }
    if (should_functioncall_wrap()) stream << ")";
}

void CodeGen_Bash::visit(const ExternCall *n) {
    if (should_functioncall_wrap()) stream << "$(";
    for (InterpolatedString::const_iterator I = n->body->begin(), E = n->body->end();
         I != E; ++I) {
        if ((*I).is_str()) {
            stream << (*I).str();
        } else {
            assert((*I).is_var());
            visit((*I).var());
        }
    }
    if (should_functioncall_wrap()) stream << ")";
}

void CodeGen_Bash::visit(const Assignment *n) {
    stream << lookup_name(n->variable) << "=";
    enable_functioncall_wrap();
    n->value->accept(this);
    disable_functioncall_wrap();
}

void CodeGen_Bash::visit(const BinOp *n) {
    bool comparison = false;
    switch (n->op) {
    case BinOp::Eq:
    case BinOp::NotEq:
    case BinOp::LT:
    case BinOp::LTE:
    case BinOp::GT:
    case BinOp::GTE:
        comparison = true;
        break;
    case BinOp::Add:
    case BinOp::Sub:
    case BinOp::Mul:
    case BinOp::Div:
        comparison = false;
        break;
    }
        
    if (!comparison) stream << "$((";
    disable_quote_variable();
    n->a->accept(this);
    switch (n->op) {
    case BinOp::Eq:
        stream << " -eq ";
        break;
    case BinOp::NotEq:
        stream << " -ne ";
        break;
    case BinOp::LT:
        stream << " -lt ";
        break;
    case BinOp::LTE:
        stream << " -lte ";
        break;
    case BinOp::GT:
        stream << " -gt ";
        break;
    case BinOp::GTE:
        stream << " -gte ";
        break;
    case BinOp::Add:
        stream << " + ";
        break;
    case BinOp::Sub:
        stream << " - ";
        break;
    case BinOp::Mul:
        stream << " * ";
        break;
    case BinOp::Div:
        stream << " / ";
        break;
    }
    n->b->accept(this);
    if (!comparison) stream << "))";
    enable_quote_variable();
}

void CodeGen_Bash::visit(const UnaryOp *n) {
    switch (n->op) {
    case UnaryOp::Negate:
        stream << "-";
        break;
    }
    n->a->accept(this);
}

void CodeGen_Bash::visit(const Integer *n) {
    stream << n->value;
}

void CodeGen_Bash::visit(const Fractional *n) {
    stream << n->value;
}

void CodeGen_Bash::visit(const String *n) {
    stream << "\"" << n->value << "\"";
}

void CodeGen_Bash::visit(const Boolean *n) {
    stream << n->value;
}
