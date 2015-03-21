#include <vector>
#include "IRAncestorsPass.h"

using namespace Bish;

void IRAncestorsPass::visit(Module *node) {
    module_stack.push(node);
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    module_stack.pop();
}

void IRAncestorsPass::visit(Block *node) {
    block_stack.push(node);
    for (std::vector<IRNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    block_stack.pop();
    node->set_parent(function_stack.top());
}

void IRAncestorsPass::visit(ReturnStatement *node) {
    node->value->accept(this);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(IfStatement *node) {
    node->pblock->condition->accept(this);
    node->pblock->body->accept(this);
    for (std::vector<PredicatedBlock *>::const_iterator I = node->elses.begin(),
             E = node->elses.end(); I != E; ++I) {
        (*I)->condition->accept(this);
        (*I)->body->accept(this);
    }
    if (node->elseblock) node->elseblock->accept(this);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(ForLoop *node) {
    node->variable->accept(this);
    node->lower->accept(this);
    if (node->upper) node->upper->accept(this);
    node->body->accept(this);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(Function *node) {
    function_stack.push(node);
    for (std::vector<Variable *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    if (node->body) node->body->accept(this);
    node->set_parent(module_stack.top());
    function_stack.pop();
}

void IRAncestorsPass::visit(FunctionCall *node) {
    for (std::vector<IRNode *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(ExternCall *node) {
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(Assignment *node) {
    node->variable->accept(this);
    node->value->accept(this);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(BinOp *node) {
    node->a->accept(this);
    node->b->accept(this);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(UnaryOp *node) {
    node->a->accept(this);
    node->set_parent(block_stack.top());
}
