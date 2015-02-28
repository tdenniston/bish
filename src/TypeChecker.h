#ifndef __BISH_TYPE_CHECKER_H__
#define __BISH_TYPE_CHECKER_H__

#include "ASTVisitor.h"
#include "SymbolTable.h"

namespace Bish {

class TypeChecker : public ASTVisitor {
public:
    TypeChecker() : current(NULL) {}
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const Assignment *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
private:
    SymbolTable *current;
};

}
#endif
