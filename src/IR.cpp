#include <cassert>
#include <iostream>
#include "CallGraph.h"
#include "FindCalls.h"
#include "IR.h"

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

Function *Module::get_function(const std::string &name) const {
    for (std::vector<Function *>::const_iterator I = functions.begin(),
             E = functions.end(); I != E; ++I) {
        if (name.compare((*I)->name) == 0) {
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
    
    std::set<std::string> to_link = find.functions();
    for (std::set<std::string>::iterator I = to_link.begin(), E = to_link.end(); I != E; ++I) {
        Function *f = m->get_function(*I);
        add_function(f);
        // Make sure to pull in functions that f calls as well.
        std::vector<Function *> calls = cg.transitive_calls(f);
        for (std::vector<Function *>::iterator CI = calls.begin(), CE = calls.end(); CI != CE; ++CI) {
            if (to_link.count((*CI)->name)) continue;
            add_function(*CI);
        }
    }
}

}
