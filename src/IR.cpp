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
    global_variables->nodes.push_back(a);
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
    std::map<Name, Function *> linked;
    for (std::set<Name>::iterator I = to_link.begin(), E = to_link.end(); I != E; ++I) {
        const Name &name = *I;
        // FindCallsToModule only compares function names to allow the
        // standard library functions to be called without a
        // namespace. Therefore, to_link can contain functions with
        // the same name but belonging to a different namespace. Don't
        // process those here:
        if (!name.namespace_id.empty() && !name.has_namespace(m->namespace_id)) continue;
        Function *f = m->get_function(name);
        assert(f);
        assert(f->name.namespace_id.empty());
        f->name.add_namespace(m->namespace_id);
        add_function(f);
        linked[f->name] = f;
        // Make sure to pull in functions that f calls as well.
        std::vector<Function *> calls = cg.transitive_calls(f);
        for (std::vector<Function *>::iterator CI = calls.begin(), CE = calls.end(); CI != CE; ++CI) {
            f = *CI;
            // Avoid dummy functions and duplicates.
            if (f->body == NULL || to_link.count(f->name) || linked.count(f->name)) continue;
            f->name.add_namespace(m->namespace_id);
            add_function(f);
            linked[f->name] = f;
        }
    }

    // Now patch up the function pointers, replacing the "dummy"
    // functions inserted at parse time with the real functions from
    // the imported module.
    std::vector<FunctionCall *> calls = find.function_calls();
    std::set<Function *> to_erase;
    for (std::vector<FunctionCall *>::iterator I = calls.begin(), E = calls.end(); I != E; ++I) {
        FunctionCall *call = *I;
        Name &name = call->function->name;
        // Special case for stdlib functions: they can be called
        // without a namespace, so add it here.
        if (m->path == get_stdlib_path()) {
            if (!name.has_namespace("stdlib")) name.add_namespace("stdlib");
        }
        if (linked.find(name) != linked.end()) {
            assert(call->function->body == NULL);
            to_erase.insert(call->function);
            call->function = linked[name];
            assert(call->function->body != NULL);
        }
    }

    // Finally, erase the old dummy functions.
    for (std::vector<Function *>::iterator I = functions.begin(), E = functions.end(); I != E; ) {
        if (to_erase.find(*I) != to_erase.end()) {
            delete *I;
            I = functions.erase(I);
        } else {
            ++I;
        }
    }
}

Type get_primitive_type(const IRNode *n) {
    if (const Integer *v = dynamic_cast<const Integer*>(n)) {
        return Type::Integer();
    } else if (const Fractional *v = dynamic_cast<const Fractional*>(n)) {
        return Type::Fractional();
    } else if (const String *v = dynamic_cast<const String*>(n)) {
        return Type::String();
    } else if (const Boolean *v = dynamic_cast<const Boolean*>(n)) {
        return Type::Boolean();
    } else {
        return Type::Undef();
    }
}

std::ostream &operator<<(std::ostream &os, const IRDebugInfo &a) {
    os << a.str();
    return os;
}

}
