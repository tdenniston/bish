#include "FindCalls.h"

using namespace Bish;

FindCallsToModule::FindCallsToModule(Module *m) {
    for (std::vector<Function *>::iterator I = m->functions.begin(),
             E = m->functions.end(); I != E; ++I) {
        Function *f = *I;
        // Skip calls to the module's main function.
        if (f == m->main) continue;
        to_find.insert(f->name);
    }
}

std::set<Name> FindCallsToModule::functions() const {
    return calls;
}

void FindCallsToModule::visit(FunctionCall *call) {
    for (std::vector<IRNode *>::const_iterator I = call->args.begin(),
             E = call->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    if (to_find.count(call->function->name)) {
        calls.insert(call->function->name);
    }
}
