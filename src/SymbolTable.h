#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

#include <map>
#include "ASTVisitor.h"
#include "Type.h"

namespace Bish {

class SymbolTableEntry {
public:
    Type type;
    SymbolTableEntry(Type ty) : type(ty) {}
};

class SymbolTable {
public:
    void insert(const ASTNode *v, Type ty);
    SymbolTableEntry *lookup(const ASTNode *v) const;
    void propagate(const ASTNode *a, const ASTNode *b);
    bool contains(const ASTNode *v) const;
private:
    // Used to map ASTNodes to entries.
    std::map<const ASTNode *, SymbolTableEntry *> ast_table;
    // Used to map names to entries.
    std::map<std::string, SymbolTableEntry *> name_table;
};

}
#endif
