#ifndef __BISH_TYPE_CHECKER_H__
#define __BISH_TYPE_CHECKER_H__

#include <set>
#include "IRVisitor.h"

namespace Bish {

class TypeChecker : public IRVisitor {
public:
    virtual void visit(Module *);
    virtual void visit(Location *);
    virtual void visit(ReturnStatement *);
    virtual void visit(ForLoop *);
    virtual void visit(FunctionCall *);
    virtual void visit(ExternCall *);
    virtual void visit(IORedirection *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
    virtual void visit(Integer *);
    virtual void visit(Fractional *);
    virtual void visit(String *);
    virtual void visit(Boolean *);
private:
    std::set<IRNode *> visited_set;
    void propagate_if_undef(IRNode *a, IRNode *b);
    bool visited(IRNode *n) { return visited_set.find(n) != visited_set.end(); }
};

}
#endif
