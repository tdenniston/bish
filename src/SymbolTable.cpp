#include <iostream>
#include <cassert>
#include "SymbolTable.h"

using namespace Bish;

void SymbolTable::insert(const std::string &v, IRNode *n, Type ty) {
    table[v] = new SymbolTableEntry(n, ty);
}

void SymbolTable::remove(const std::string &v) {
    table.erase(v);
}

SymbolTableEntry *SymbolTable::lookup(const std::string &v) const {
    std::map<std::string, SymbolTableEntry *>::const_iterator I = table.find(v);
    if (I == table.end()) {
        if (parent) {
            return parent->lookup(v);
        } else {
            return NULL;
        }
    }
    return I->second;
}

void SymbolTable::propagate(const std::string &a, const std::string &b) {
    SymbolTableEntry *e = lookup(b);
    if (e) {
        insert(a, e->node, e->type);
    }
}

bool SymbolTable::contains(const std::string &v) const {
    return table.find(v) != table.end();
}
