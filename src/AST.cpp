#include <iostream>
#include "AST.h"

namespace Bish {

void BishPrinter::visit(const Block *n) {
    stream << "{\n";
    for (std::vector<ASTNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        stream << "    ";
        (*I)->accept(this);
        stream << "\n";
    }
    stream << "}\n";
}

void BishPrinter::visit(const Variable *n) {
    stream << n->name;
}

void BishPrinter::visit(const Assignment *n) {
    n->variable->accept(this);
    stream << " = ";
    n->value->accept(this);
    stream << ";";
}

void BishPrinter::visit(const BinOp *n) {
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
}

void BishPrinter::visit(const UnaryOp *n) {
    switch (n->op) {
    case UnaryOp::Negate:
        stream << "-";
        break;
    }
    n->a->accept(this);
}

void BishPrinter::visit(const Integer *n) {
    stream << n->value;
}

void BishPrinter::visit(const Fractional *n) {
    stream << n->value;
}

void BishPrinter::visit(const String *n) {
    stream << n->value;
}

void BishPrinter::visit(const Boolean *n) {
    stream << n->value;
}

}
