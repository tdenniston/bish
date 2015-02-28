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
