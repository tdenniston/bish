#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

#include <map>
#include "IR.h"

namespace Bish {

class SymbolTableEntry {
public:
    IRNode *node;
    SymbolTableEntry(IRNode *n) : node(n) {}
};

class SymbolTable {
public:
    SymbolTable() : parent(NULL) {}
    SymbolTable(SymbolTable *p) : parent(p) {}
    void insert(const Name &name, IRNode *n);
    void remove(const Name &name);
    SymbolTableEntry *lookup(const Name &name) const;
    bool contains(const Name &v) const;
private:
    std::map<Name, SymbolTableEntry *> table;
    SymbolTable *parent;
};

}
#endif
