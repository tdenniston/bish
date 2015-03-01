#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

#include <map>
#include "Type.h"

namespace Bish {

class SymbolTableEntry {
public:
    Type type;
    SymbolTableEntry(Type ty) : type(ty) {}
};

class SymbolTable {
public:
    SymbolTable() : parent(NULL) {}
    SymbolTable(SymbolTable *p) : parent(p) {}
    void insert(const std::string &name, Type ty);
    SymbolTableEntry *lookup(const std::string &name) const;
    void propagate(const std::string &a, const std::string &b);
    bool contains(const std::string &v) const;
private:
    std::map<std::string, SymbolTableEntry *> table;
    SymbolTable *parent;
};

}
#endif
