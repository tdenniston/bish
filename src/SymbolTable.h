#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

#include <map>
#include "ASTVisitor.h"

namespace Bish {

class SymbolTable {
public:
    void insert(Variable *v, ASTNode *value);
    ASTNode *lookup(Variable *v) const;
private:
    typedef std::map<std::string, ASTNode *> TableTy;
    typedef TableTy::iterator iterator;
    typedef TableTy::const_iterator const_iterator;
    TableTy table;
};

// ASTVisitor that creates symbol tables.
class CreateSymbolTable : public ASTVisitor {
public:
    CreateSymbolTable() : current(NULL) {}
    virtual void visit(const Block *);
    virtual void visit(const Assignment *);
private:
    SymbolTable *current;
};

}
#endif
