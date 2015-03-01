#ifndef __BISH_TYPE_CHECKER_H__
#define __BISH_TYPE_CHECKER_H__

#include "ASTVisitor.h"
#include "SymbolTable.h"

namespace Bish {

class ASTNodeSymbolTable {
public:
    void insert(const ASTNode *node, Type ty);
    SymbolTableEntry *lookup(const ASTNode *node) const;
private:
    std::map<const ASTNode *, SymbolTableEntry *> table;
};

class TypeChecker : public ASTVisitor {
public:
    TypeChecker() : current_symtab(NULL), current_astnode_symtab(NULL) {}
    virtual void visit(const Block *);
    virtual void visit(const Assignment *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
    virtual void visit(const Integer *);
    virtual void visit(const Fractional *);
    virtual void visit(const String *);
    virtual void visit(const Boolean *);
private:
    SymbolTable *current_symtab;
    ASTNodeSymbolTable *current_astnode_symtab;

    SymbolTableEntry *lookup(const ASTNode *n);
};

}
#endif
