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
    stream << "$" << lookup_name(n);
}

void CodeGen_Bash::visit(const IfStatement *n) {
    stream << "if ";
    stream << "[[ ";
    n->condition->accept(this);
    stream << " ]]";
    stream << "; then\n";
    disable_block_braces();
    n->body->accept(this);
    enable_block_braces();
    indent();
    stream << "fi";
}

void CodeGen_Bash::visit(const ForLoop *n) {
    stream << "for " << lookup_name(n->variable) << " in ";
    for (int i = n->lower->value; i < n->upper->value; i++) {
      stream << i << " ";
    }
    stream << "; do\n";
    disable_block_braces();
    n->body->accept(this);
    enable_block_braces();
    indent();
    stream << "done";
}

void CodeGen_Bash::visit(const Function *n) {
    stream << "function " << n->name << " ";
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
    stream << n->name;
    for (int i = 0; i < nargs; i++) {
        stream << " ";
        n->args[i]->accept(this);
    }
}

void CodeGen_Bash::visit(const ExternCall *n) {
    for (InterpolatedString::const_iterator I = n->body->begin(), E = n->body->end();
         I != E; ++I) {
        if ((*I).is_str()) {
            stream << (*I).str();
        } else {
            assert((*I).is_var());
            visit((*I).var());
        }
    }
}

void CodeGen_Bash::visit(const Comparison *n) {
    n->a->accept(this);
    stream << " == ";
    n->b->accept(this);
}

void CodeGen_Bash::visit(const Assignment *n) {
    stream << lookup_name(n->variable) << "=";
    n->value->accept(this);
}

void CodeGen_Bash::visit(const BinOp *n) {
    stream << "$((";
    n->a->accept(this);
    switch (n->op) {
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
    stream << "))";
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
