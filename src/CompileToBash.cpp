#include "CompileToBash.h"

using namespace Bish;

void CompileToBash::visit(const Block *n) {
    indent_level++;
    for (std::vector<ASTNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        for (unsigned i = 0; i < indent_level - 1; i++) {
            stream << "    ";
        }
        (*I)->accept(this);
        stream << "\n";
    }
    indent_level--;
}

void CompileToBash::visit(const Variable *n) {
    stream << "$" << n->name;
}

void CompileToBash::visit(const IfStatement *n) {
    stream << "if ";
    stream << "[[ ";
    n->condition->accept(this);
    stream << " ]]";
    stream << "; then\n";
    n->body->accept(this);
    stream << "fi";
}

void CompileToBash::visit(const Comparison *n) {
    n->a->accept(this);
    stream << " == ";
    n->b->accept(this);
}

void CompileToBash::visit(const Assignment *n) {
    stream << n->variable->name << "=";
    n->value->accept(this);
    stream << ";";
}

void CompileToBash::visit(const BinOp *n) {
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

void CompileToBash::visit(const UnaryOp *n) {
    switch (n->op) {
    case UnaryOp::Negate:
        stream << "-";
        break;
    }
    n->a->accept(this);
}

void CompileToBash::visit(const Integer *n) {
    stream << n->value;
}

void CompileToBash::visit(const Fractional *n) {
    stream << n->value;
}

void CompileToBash::visit(const String *n) {
    stream << "\"" << n->value << "\"";
}

void CompileToBash::visit(const Boolean *n) {
    stream << n->value;
}
