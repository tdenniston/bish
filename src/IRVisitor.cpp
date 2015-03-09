#include "IRVisitor.h"
#include "IR.h"

using namespace Bish;

IRVisitor::~IRVisitor() { }

void IRVisitor::visit(Module *node) {
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(Block *node) {
    for (std::vector<IRNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(Variable *node) {

}

void IRVisitor::visit(ReturnStatement *node) {
    node->value->accept(this);
}

void IRVisitor::visit(IfStatement *node) {
    node->pblock->condition->accept(this);
    node->pblock->body->accept(this);
    for (std::vector<PredicatedBlock *>::const_iterator I = node->elses.begin(),
             E = node->elses.end(); I != E; ++I) {
        (*I)->condition->accept(this);
        (*I)->body->accept(this);
    }
    if (node->elseblock) node->elseblock->accept(this);
}

void IRVisitor::visit(ForLoop *node) {
    node->variable->accept(this);
    node->lower->accept(this);
    if (node->upper) node->upper->accept(this);
    node->body->accept(this);
}

void IRVisitor::visit(Function *node) {
    for (std::vector<Variable *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    node->body->accept(this);
}

void IRVisitor::visit(FunctionCall *node) {
    for (std::vector<IRNode *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(ExternCall *node) {

}

void IRVisitor::visit(Assignment *node) {
    node->variable->accept(this);
    node->value->accept(this);
}

void IRVisitor::visit(BinOp *node) {
    node->a->accept(this);
    node->b->accept(this);
}

void IRVisitor::visit(UnaryOp *node) {
    node->a->accept(this);
}

void IRVisitor::visit(Integer *node) {

}

void IRVisitor::visit(Fractional *node) {

}

void IRVisitor::visit(String *node) {

}

void IRVisitor::visit(Boolean *node) {

}

