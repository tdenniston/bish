#ifndef __BISH_TYPE_CHECKER_H__
#define __BISH_TYPE_CHECKER_H__

#include "IRVisitor.h"
#include "SymbolTable.h"

namespace Bish {

class IRNodeSymbolTable {
public:
    void insert(const IRNode *node, Type ty);
    SymbolTableEntry *lookup(const IRNode *node) const;
private:
    std::map<const IRNode *, SymbolTableEntry *> table;
};

class TypeChecker : public IRVisitor {
public:
    TypeChecker() : current_symtab(NULL), current_astnode_symtab(NULL) {}
    virtual void visit(const Block *);
    virtual void visit(const Assignment *);
    virtual void visit(const IfStatement *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
    virtual void visit(const Integer *);
    virtual void visit(const Fractional *);
    virtual void visit(const String *);
    virtual void visit(const Boolean *);
private:
    SymbolTable *current_symtab;
    IRNodeSymbolTable *current_astnode_symtab;

    SymbolTableEntry *lookup(const IRNode *n,
                             bool assert_non_null=true);
};

}
#endif
