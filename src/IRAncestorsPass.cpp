#include <vector>
#include "IRAncestorsPass.h"

using namespace Bish;

void IRAncestorsPass::visit(Module *node) {
    module_stack.push(node);
    // Add dummy root-level block.
    block_stack.push(new Block());
    IRVisitor::visit(node);
    block_stack.pop();
    module_stack.pop();
}

void IRAncestorsPass::visit(Block *node) {
    block_stack.push(node);
    IRVisitor::visit(node);
    block_stack.pop();
    node->set_parent(function_stack.top());
}

void IRAncestorsPass::visit(ReturnStatement *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(IfStatement *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(ForLoop *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(Function *node) {
    function_stack.push(node);
    IRVisitor::visit(node);
    node->set_parent(module_stack.top());
    function_stack.pop();
}

void IRAncestorsPass::visit(FunctionCall *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(ExternCall *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(Assignment *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(BinOp *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}

void IRAncestorsPass::visit(UnaryOp *node) {
    IRVisitor::visit(node);
    node->set_parent(block_stack.top());
}
