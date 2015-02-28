#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

#include <map>

namespace Bish {

class ASTNode;
class Variable;

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

}
#endif
