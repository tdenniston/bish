#ifndef __BISH_CALL_GRAPH_H__
#define __BISH_CALL_GRAPH_H__

#include <map>
#include <vector>
#include "IR.h"
#include "IRVisitor.h"

namespace Bish {

class CallGraph {
    friend class CallGraphBuilder;
public:
    const std::vector<Function *> &calls(Function *f) const;
    std::vector<Function *> transitive_calls(Function *root) const;
    const std::vector<Function *> &callers(Function *f) const;
private:
    typedef std::vector<Function *> FuncVec;
    typedef std::map<Function *, FuncVec> FuncMap;
    FuncMap calls_map;
    FuncMap callers_map;
};

class CallGraphBuilder : public IRVisitor {
public:
    CallGraph build(Module *m);
private:
    CallGraph cg;
    virtual void visit(Function *f);
    virtual void visit(FunctionCall *call);
};

}

#endif
