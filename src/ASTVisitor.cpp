#include "ASTVisitor.h"
#include "AST.h"

using namespace Bish;

ASTVisitor::~ASTVisitor() { }

void ASTVisitor::visit(const Block *node) {
    for (std::vector<ASTNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void ASTVisitor::visit(const Variable *node) {

}

void ASTVisitor::visit(const NameList *node) {
    for (std::vector<Variable *>::const_iterator I = node->variables.begin(),
             E = node->variables.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void ASTVisitor::visit(const IfStatement *node) {
    node->condition->accept(this);
    node->body->accept(this);
}

void ASTVisitor::visit(const Function *node) {
    node->name->accept(this);
    node->args->accept(this);
    node->body->accept(this);
}

void ASTVisitor::visit(const Comparison *node) {
    node->a->accept(this);
    node->b->accept(this);
}

void ASTVisitor::visit(const Assignment *node) {
    node->variable->accept(this);
    node->value->accept(this);
}

void ASTVisitor::visit(const BinOp *node) {
    node->a->accept(this);
    node->b->accept(this);
}

void ASTVisitor::visit(const UnaryOp *node) {
    node->a->accept(this);
}

void ASTVisitor::visit(const Integer *node) {

}

void ASTVisitor::visit(const Fractional *node) {

}

void ASTVisitor::visit(const String *node) {

}

void ASTVisitor::visit(const Boolean *node) {

}

