#ifndef __BISH_LINK_IMPORTS_PASS_H__
#define __BISH_LINK_IMPORTS_PASS_H__

#include "IR.h"
#include "IRVisitor.h"

namespace Bish {

/* This pass performs the linking of modules specified by import
 * statements. Each Module maintains a list of "external" functions
 * (functions belonging to other modules). This pass parses modules
 * specified with import statements, determines which functions are
 * needed by the calling module, and adds those functions to the
 * calling module's list of external functions. */
class LinkImportsPass : public IRVisitor {
public:
    virtual void visit(Module *);
    virtual void visit(ImportStatement *);
private:
    Module *module;
};

}

#endif
