#ifndef __BISH_FIND_CALLS_H__
#define __BISH_FIND_CALLS_H__

#include <string>
#include <set>
#include "IR.h"

namespace Bish {

// Find all calls to functions in a specified module.
// Example:
//     FindCallsToModule find(m1);
//     m2->accept(&find);
// This returns a list of functions in m1 called by m2:
//     find.functions();
class FindCallsToModule : public IRVisitor {
public:
    FindCallsToModule(Module *m);
    std::set<Name> functions() const;
    virtual void visit(FunctionCall *call);
private:
    std::set<Name> to_find;
    std::set<Name> calls;
};

}

#endif
