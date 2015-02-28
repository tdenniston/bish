#include <iostream>
#include <cassert>
#include "AST.h"
#include "SymbolTable.h"

using namespace Bish;

void SymbolTable::insert(const ASTNode *v, Type ty) {
    assert(v);
    SymbolTableEntry *e = new SymbolTableEntry(ty);
    if (const Variable *var = dynamic_cast<const Variable*>(v)) {
        name_table[var->name] = e;
    } else {
        ast_table[v] = e;
    }
}

SymbolTableEntry *SymbolTable::lookup(const ASTNode *v) const {
    assert(v);
    if (const Variable *var = dynamic_cast<const Variable*>(v)) {
        std::map<std::string, SymbolTableEntry *>::const_iterator I = name_table.find(var->name);
        if (I == name_table.end()) {
            return NULL;
        }
        return I->second;
    } else {
        std::map<const ASTNode *, SymbolTableEntry *>::const_iterator I = ast_table.find(v);
        if (I == ast_table.end()) {
            return NULL;
        }
        return I->second;
    }
}

void SymbolTable::propagate(const ASTNode *a, const ASTNode *b) {
    assert(a && b);
    SymbolTableEntry *e = lookup(b);
    if (e) {
        insert(a, e->type);
    }
}

bool SymbolTable::contains(const ASTNode *v) const {
    assert(v);
    if (const Variable *var = dynamic_cast<const Variable*>(v)) {
        return name_table.find(var->name) != name_table.end();
    } else {
        return ast_table.find(v) != ast_table.end();
    }
}
