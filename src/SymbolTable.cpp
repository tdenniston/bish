#include "AST.h"
#include "SymbolTable.h"

using namespace Bish;

void SymbolTable::insert(Variable *v, ASTNode *value) {
    table[v->name] = value;
}

ASTNode *SymbolTable::lookup(Variable *v) const {
    const_iterator I = table.find(v->name);
    if (I == table.end()) {
        return NULL;
    }
    return I->second;
}

void CreateSymbolTable::visit(const Block *node) {
    SymbolTable *old = current;
    current = node->symbol_table;
    for (std::vector<ASTNode *>::const_iterator I = node->nodes.begin(),
             E = node->nodes.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    current = old;
}

void CreateSymbolTable::visit(const Assignment *node) {
    current->insert(node->variable, node->value);
}
