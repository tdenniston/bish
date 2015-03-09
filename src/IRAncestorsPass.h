#ifndef __BISH_IR_ANCESTORS_PASS_H__
#define __BISH_IR_ANCESTORS_PASS_H__

#include <stack>
#include "IR.h"
#include "IRVisitor.h"

namespace Bish {

class IRAncestorsPass : public IRVisitor {
public:
    virtual void visit(Module *);
    virtual void visit(Block *);
    virtual void visit(Function *);
    virtual void visit(FunctionCall *);
    virtual void visit(ExternCall *);
    virtual void visit(IfStatement *);
    virtual void visit(ReturnStatement *);
    virtual void visit(ForLoop *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
private:
    std::stack<Module *> module_stack;
    std::stack<Function *> function_stack;
    std::stack<Block *> block_stack;
};

};

#endif
