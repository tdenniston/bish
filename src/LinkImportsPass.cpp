#include "Parser.h"
#include "LinkImportsPass.h"

using namespace Bish;

void LinkImportsPass::visit(Module *node) {
    module = node;
    for (std::vector<Assignment *>::const_iterator I = node->global_variables.begin(),
             E = node->global_variables.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void LinkImportsPass::visit(ImportStatement *node) {
    Parser p;
    Module *m = p.parse(node->path);
    module->import(m);
}
