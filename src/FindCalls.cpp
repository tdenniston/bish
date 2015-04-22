#include "FindCalls.h"

using namespace Bish;

FindCallsToModule::FindCallsToModule(Module *m) {
    for (std::vector<Function *>::iterator I = m->functions.begin(),
             E = m->functions.end(); I != E; ++I) {
        Function *f = *I;
        // Skip calls to the module's main function.
        if (f == m->main) continue;
        to_find.insert(f->name.name);
    }
}

std::set<Name> FindCallsToModule::functions() const {
    return calls;
}

std::vector<FunctionCall *> FindCallsToModule::function_calls() const {
    return fcalls;
}

void FindCallsToModule::visit(FunctionCall *call) {
    IRVisitor::visit(call);
    if (to_find.count(call->function->name.name)) {
        calls.insert(call->function->name);
        fcalls.push_back(call);
    }
}
