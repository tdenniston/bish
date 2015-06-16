#include <iostream>
#include <cassert>
#include "SymbolTable.h"

using namespace Bish;

void SymbolTable::insert(const Name &v, IRNode *n) {
    table[v] = new SymbolTableEntry(n);
}

void SymbolTable::remove(const Name &v) {
    table.erase(v);
}

SymbolTableEntry *SymbolTable::lookup(const Name &v) const {
    std::map<Name, SymbolTableEntry *>::const_iterator I = table.find(v);
    if (I == table.end()) {
        if (parent) {
            return parent->lookup(v);
        } else {
            return NULL;
        }
    }
    return I->second;
}

bool SymbolTable::contains(const Name &v) const {
    return table.find(v) != table.end();
}
