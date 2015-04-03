#ifndef __BISH_BY_REFERENCE_PASS_H__
#define __BISH_BY_REFERENCE_PASS_H__

#include "IR.h"
#include "IRVisitor.h"
#include <map>

namespace Bish {

/** This pass performs any IR modifications needed to ensure that some
 * function parameters are passed by reference. Currently, this is
 * only used for arrays, which must be passed by reference. The
 * mechanism currently used for pass-by-reference is using a unique
 * global variable to communicate between functions using the value,
 * instead of a function parameter. */
class ByReferencePass : public IRVisitor {
public:
    virtual void visit(Module *);
    virtual void visit(FunctionCall *);
private:
    unsigned unique_id;
    std::set<Name> used_names;
    std::map<Function *, std::map<unsigned, Variable *> > reference_vars;
    Name get_unique_name();
    void initialize_unique_naming(Module *m);
};

}

#endif
