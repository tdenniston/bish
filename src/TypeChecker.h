#ifndef __BISH_TYPE_CHECKER_H__
#define __BISH_TYPE_CHECKER_H__

#include "IRVisitor.h"

namespace Bish {

class TypeChecker : public IRVisitor {
public:
    virtual void visit(ReturnStatement *);
    virtual void visit(Function *);
    virtual void visit(FunctionCall *);
    virtual void visit(ExternCall *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
    virtual void visit(Integer *);
    virtual void visit(Fractional *);
    virtual void visit(String *);
    virtual void visit(Boolean *);
private:
    void propagate_if_undef(IRNode *a, IRNode *b);
};

}
#endif
