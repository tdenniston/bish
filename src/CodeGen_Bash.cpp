#include "CodeGen_Bash.h"

using namespace Bish;

void CodeGen_Bash::visit(const Module *n) {
    for (std::vector<Function *>::const_iterator I = n->functions.begin(),
             E = n->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void CodeGen_Bash::visit(const Block *n) {
    if (should_print_block_braces()) stream << "{\n";
    indent_level++;
    for (std::vector<IRNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        for (unsigned i = 0; i < indent_level - 1; i++) {
            stream << "    ";
        }
        (*I)->accept(this);
        stream << ";\n";
    }
    indent_level--;
    if (should_print_block_braces()) stream << "}";
}

void CodeGen_Bash::visit(const Variable *n) {
    stream << "$" << n->name;
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
    stream << "fi";
}

void CodeGen_Bash::visit(const Function *n) {
    stream << "function " << n->name << " ";
    // Bash doesn't allow named arguments to functions.
    // We'll have to translate to positional arguments.
    stream << "() ";
    n->body->accept(this);
}

void CodeGen_Bash::visit(const FunctionCall *n) {
    const int nargs = n->args.size();
    stream << n->name;
    for (int i = 0; i < nargs; i++) {
        stream << " ";
        n->args[i]->accept(this);
    }
}

void CodeGen_Bash::visit(const Comparison *n) {
    n->a->accept(this);
    stream << " == ";
    n->b->accept(this);
}

void CodeGen_Bash::visit(const Assignment *n) {
    stream << n->variable->name << "=";
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
