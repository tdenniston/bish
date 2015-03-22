#include <cassert>
#include <iostream>
#include "CallGraph.h"
#include "FindCalls.h"
#include "IR.h"
#include "Util.h"

namespace Bish {

void Module::set_main(Function *f) {
    add_function(f);
    main = f;
}

void Module::add_function(Function *f) {
    functions.push_back(f);
}

void Module::add_global(Assignment *a) {
    global_variables.push_back(a);
}

void Module::set_path(const std::string &p) {
    path = p;
    namespace_id = remove_suffix(basename(path), ".");
    assert(!namespace_id.empty() && "Unable to resolve namespace identifier.");
}

Function *Module::get_function(const Name &name) const {
    for (std::vector<Function *>::const_iterator I = functions.begin(),
             E = functions.end(); I != E; ++I) {
        if (name.name == (*I)->name.name) {
            return *I;
        }
    }
    return NULL;
}

void Module::import(Module *m) {
    FindCallsToModule find(m);
    accept(&find);
    CallGraphBuilder cgb;
    CallGraph cg = cgb.build(m);

    std::set<Name> to_link = find.functions();
    for (std::set<Name>::iterator I = to_link.begin(), E = to_link.end(); I != E; ++I) {
        const Name &name = *I;
        // FindCallsToModule only compares function names to allow the
        // standard library functions to be called without a
        // namespace. Therefore, to_link can contain functions with
        // the same name but belonging to a different namespace. Don't
        // process those here:
        if (!name.namespace_id.empty() && name.namespace_id != m->namespace_id) continue;
        Function *f = m->get_function(name);
        assert(f);
        assert(f->name.namespace_id.empty());
        f->name.namespace_id = m->namespace_id;
        add_function(f);
        // Make sure to pull in functions that f calls as well.
        std::vector<Function *> calls = cg.transitive_calls(f);
        for (std::vector<Function *>::iterator CI = calls.begin(), CE = calls.end(); CI != CE; ++CI) {
            f = *CI;
            // Avoid dummy functions and duplicates.
            if (f->body == NULL || to_link.count(f->name)) continue;
            assert(f->name.namespace_id.empty());
            f->name.namespace_id = m->namespace_id;
            add_function(f);
        }
    }

    // Special case for standard library functions: fix up the
    // function call namespaces. This is so that the user does not
    // have to write, for example, "Stdlib.assert()" in order to call
    // the standard library assert function.
    if (m->path == get_stdlib_path()) {
        std::vector<FunctionCall *> calls = find.function_calls();
        for (std::vector<FunctionCall *>::iterator I = calls.begin(), E = calls.end(); I != E; ++I) {
            FunctionCall *call = *I;
            call->function->name.namespace_id = "StdLib";
        }
    }
}

}
