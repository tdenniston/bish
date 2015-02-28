#ifndef __BISH_SYMBOL_TABLE_H__
#define __BISH_SYMBOL_TABLE_H__

class ASTNode;
class Variable;

class SymbolTable {
public:
    void insert(Variable *v, ASTNode *value);
    ASTNode *lookup(Variable *v);
private:
};

#endif
