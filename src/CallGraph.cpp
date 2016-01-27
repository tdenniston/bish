#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include "CallGraph.h"

using namespace Bish;

// Return a list of direct calls from f.
const std::vector<Function *> &CallGraph::calls(Function *f) {
    return calls_map[f];
}

// Return a list of all calls (recursively) from root.
std::vector<Function *> CallGraph::transitive_calls(Function *root) {
    std::vector<Function *> result;
    std::set<Function *> visited;
    std::queue<Function *> worklist;
    worklist.push(root);
    visited.insert(root);
    while (!worklist.empty()) {
        Function *f = worklist.front();
        worklist.pop();
        std::vector<Function *> direct = calls(f);
        for (std::vector<Function *>::iterator I = direct.begin(), E = direct.end(); I != E; ++I) {
            if (visited.find(*I) == visited.end()) {
                visited.insert(*I);
                worklist.push(*I);
                result.push_back(*I);
            }
        }
    }
    return result;
}

// Return a list of functions which call f.
const std::vector<Function *> &CallGraph::callers(Function *f) {
  return callers_map[f];
}

void CallGraphBuilder::visit(Function *f) {
    IRVisitor::visit(f);

    if (cg.calls_map.find(f) == cg.calls_map.end()) {
        cg.calls_map[f] = CallGraph::FuncVec();
    }
    if (cg.callers_map.find(f) == cg.callers_map.end()) {
        cg.callers_map[f] = CallGraph::FuncVec();
    }
}

void CallGraphBuilder::visit(FunctionCall *call) {
    IRVisitor::visit(call);

    Block *b = dynamic_cast<Block *>(call->parent());
    assert(b);
    Function *f = dynamic_cast<Function *>(b->parent());
    // The parent of a block can be null if the block is the Module
    // global variable block. Currently, don't add function calls from
    // the global variable initializers to the callgraph.
    if (f) {
        cg.calls_map[f].push_back(call->function);
        cg.callers_map[call->function].push_back(f);
    }
}

CallGraph CallGraphBuilder::build(Module *m) {
    m->accept(this);
    return cg;
}
