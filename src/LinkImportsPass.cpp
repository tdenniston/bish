#include "Parser.h"
#include "LinkImportsPass.h"

using namespace Bish;

void LinkImportsPass::visit(Module *node) {
    module = node;
    node->global_variables->accept(this);
    // There's probably a better way to do this. The issue is that
    // visiting functions which have import statements in them can
    // cause the list of functions in the module to change. Thus, we
    // can't simply iterate over the list at this point, as it might
    // change dynamically.
    std::set<Function *> functions(node->functions.begin(), node->functions.end());
    std::set<Function *> finished;
    while (!functions.empty()) {
        Function *f = *functions.begin();
        functions.erase(functions.begin());
        if (finished.find(f) != finished.end()) continue;
        finished.insert(f);
        f->accept(this);
        functions.insert(node->functions.begin(), node->functions.end());
    }
}

void LinkImportsPass::visit(ImportStatement *node) {
    Parser p;
    Module *m = p.parse(node->path);
    module->import(m);
}
