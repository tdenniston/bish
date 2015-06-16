#include "IRVisitor.h"
#include "IR.h"

using namespace Bish;

IRVisitor::~IRVisitor() { }

void IRVisitor::visit(Module *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    for (std::vector<Assignment *>::const_iterator I = node->global_variables.begin(),
             E = node->global_variables.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(Block *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    for (std::vector<IRNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(Variable *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(Location *node) {
    if (visited(node)) return;
    visited_set.insert(node);
    node->variable->accept(this);
    if (node->offset) node->offset->accept(this);
}

void IRVisitor::visit(ReturnStatement *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    if (node->value) node->value->accept(this);
}

void IRVisitor::visit(ImportStatement *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(LoopControlStatement *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(IfStatement *node) {
    if (visited(node)) return;
    visited_set.insert(node);

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
    if (visited(node)) return;
    visited_set.insert(node);

    node->variable->accept(this);
    node->lower->accept(this);
    if (node->upper) node->upper->accept(this);
    node->body->accept(this);
}

void IRVisitor::visit(Function *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    for (std::vector<Variable *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    if (node->body) node->body->accept(this);
}

void IRVisitor::visit(FunctionCall *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    for (std::vector<Assignment *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(ExternCall *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(IORedirection *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    node->a->accept(this);
    node->b->accept(this);
}

void IRVisitor::visit(Assignment *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    node->location->accept(this);
    for (std::vector<IRNode *>::const_iterator I = node->values.begin(),
             E = node->values.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void IRVisitor::visit(BinOp *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    node->a->accept(this);
    node->b->accept(this);
}

void IRVisitor::visit(UnaryOp *node) {
    if (visited(node)) return;
    visited_set.insert(node);

    node->a->accept(this);
}

void IRVisitor::visit(Integer *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(Fractional *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(String *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

void IRVisitor::visit(Boolean *node) {
    if (visited(node)) return;
    visited_set.insert(node);
}

