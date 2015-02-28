#include <iostream>
#include <cassert>
#include "AST.h"
#include "TypeChecker.h"

using namespace Bish;

void TypeChecker::visit(const Block *node) {
    SymbolTable *old = current;
    current = node->symbol_table;
    for (std::vector<ASTNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    current = old;
}

void TypeChecker::visit(const Variable *node) {
    // Variable symbol table entries are by name, so we must propagate the type to this AST node.
    SymbolTableEntry *e = current->lookup(node);
    // There may not be a symbol table entry for 'node' if we are on the left hand side of an assignment.
    if (e) {
        current->insert(node, e->type);
    }
}

void TypeChecker::visit(const Assignment *node) {
    node->variable->accept(this);
    node->value->accept(this);

    SymbolTableEntry *eval = current->lookup(node->value);
    assert(eval);
    current->insert(node->variable, eval->type);
}

void TypeChecker::visit(const BinOp *node) {
    node->a->accept(this);
    node->b->accept(this);

    SymbolTableEntry *ea = current->lookup(node->a);
    SymbolTableEntry *eb = current->lookup(node->b);
    assert(ea && eb);
    assert(ea->type == eb->type && "Type mismatch to binary operator.");
    current->insert(node, ea->type);
}

void TypeChecker::visit(const UnaryOp *node) {
    node->a->accept(this);
    SymbolTableEntry *e = current->lookup(node->a);
    assert(e);
    current->insert(node, e->type);
}
