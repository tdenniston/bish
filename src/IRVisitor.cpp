#include "IRVisitor.h"
#include "IR.h"

using namespace Bish;

IRVisitor::~IRVisitor() { }

void IRVisitor::visit(const Module *node) {
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(const Block *node) {
    for (std::vector<IRNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(const Variable *node) {

}

void IRVisitor::visit(const IfStatement *node) {
    node->condition->accept(this);
    node->body->accept(this);
}

void IRVisitor::visit(const ForLoop *node) {
    node->variable->accept(this);
    node->lower->accept(this);
    node->upper->accept(this);
    node->body->accept(this);
}

void IRVisitor::visit(const Function *node) {
    for (std::vector<Variable *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    node->body->accept(this);
}

void IRVisitor::visit(const FunctionCall *node) {
    for (std::vector<IRNode *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(const ExternCall *node) {

}

void IRVisitor::visit(const Comparison *node) {
    node->a->accept(this);
    node->b->accept(this);
}

void IRVisitor::visit(const Assignment *node) {
    node->variable->accept(this);
    node->value->accept(this);
}

void IRVisitor::visit(const BinOp *node) {
    node->a->accept(this);
    node->b->accept(this);
}

void IRVisitor::visit(const UnaryOp *node) {
    node->a->accept(this);
}

void IRVisitor::visit(const Integer *node) {

}

void IRVisitor::visit(const Fractional *node) {

}

void IRVisitor::visit(const String *node) {

}

void IRVisitor::visit(const Boolean *node) {

}

