#include <iostream>
#include "AST.h"

namespace Bish {

void ASTPrinter::print(AST *ast) {
    ASTNode *root = ast->root();
    print(root, std::cout);
}

void ASTPrinter::print(ASTNode *node, std::ostream &os) {
    if (Block *b = dynamic_cast<Block*>(node)) {
        print_block(b, os);
    } else if (Variable *n = dynamic_cast<Variable*>(node)) {
        print_variable(n, os);
    } else if (Assignment *n = dynamic_cast<Assignment*>(node)) {
        print_assignment(n, os);
    } else if (BinOp *n = dynamic_cast<BinOp*>(node)) {
        print_binop(n, os);
    } else if (UnaryOp *n = dynamic_cast<UnaryOp*>(node)) {
        print_unaryop(n, os);
    } else if (Integer *n = dynamic_cast<Integer*>(node)) {
        print_integer(n, os);
    } else if (Fractional *n = dynamic_cast<Fractional*>(node)) {
        print_fractional(n, os);
    } else if (String *n = dynamic_cast<String*>(node)) {
        print_string(n, os);
    } else if (Boolean *n = dynamic_cast<Boolean*>(node)) {
        print_boolean(n, os);
    } else {
        os << "??";
    }
}

void BishPrinter::print_block(Block *n, std::ostream &os) {
    os << "{\n";
    for (std::vector<ASTNode *>::iterator I = n->nodes.begin(), E = n->nodes.end();
         I != E; ++I) {
        os << "    ";
        print(*I, os);
        os << "\n";
    }
    os << "}\n";
}

void BishPrinter::print_variable(Variable *n, std::ostream &os) {
    os << n->name;
}

void BishPrinter::print_assignment(Assignment *n, std::ostream &os) {
    print(n->variable, os);
    os << " = ";
    print(n->value, os);
    os << ";";
}

void BishPrinter::print_binop(BinOp *n, std::ostream &os) {
    print(n->a, os);
    switch (n->op) {
    case BinOp::Add:
        os << " + ";
        break;
    case BinOp::Sub:
        os << " - ";
        break;
    case BinOp::Mul:
        os << " * ";
        break;
    case BinOp::Div:
        os << " / ";
        break;
    }
    print(n->b, os);
}

void BishPrinter::print_unaryop(UnaryOp *n, std::ostream &os) {
    switch (n->op) {
    case UnaryOp::Negate:
        os << "-";
        break;
    }
    print(n->a, os);
}

void BishPrinter::print_integer(Integer *n, std::ostream &os) {
    os << n->value;
}

void BishPrinter::print_fractional(Fractional *n, std::ostream &os) {
    os << n->value;
}

void BishPrinter::print_string(String *n, std::ostream &os) {
    os << n->value;
}

void BishPrinter::print_boolean(Boolean *n, std::ostream &os) {
    os << n->value;
}

}
