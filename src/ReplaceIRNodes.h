#ifndef __BISH_REPLACE_IR_NODES_H__
#define __BISH_REPLACE_IR_NODES_H__

#include "IRVisitor.h"
#include <cassert>
#include <map>

namespace Bish {

// Replaces IRNodes with other IRNodes.
class ReplaceIRNodes : public IRVisitor {
public:
    template <class S, class T>
    ReplaceIRNodes(const std::map<S*, T*> &replace) {
        typename std::map<S*, T*>::const_iterator I, E;
        for (I = replace.begin(), E = replace.end(); I != E; ++I) {
            IRNode *a = dynamic_cast<IRNode*>(I->first),
                *b = dynamic_cast<IRNode*>(I->second);
            assert(I->first && I->second);
            assert(a && b);
            replace_map[a] = b;
        }
    }

    virtual void visit(Module *);
    virtual void visit(Block *);
    virtual void visit(FunctionCall *);
    virtual void visit(IORedirection *);
    virtual void visit(IfStatement *);
    virtual void visit(ReturnStatement *);
    virtual void visit(ForLoop *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
private:
    std::map<IRNode *, IRNode *> replace_map;
    IRNode *replacement(IRNode *node);
};

}
#endif
