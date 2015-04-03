#include "ByReferencePass.h"

using namespace Bish;

void ByReferencePass::initialize_unique_naming(Module *m) {
    unique_id = 0;
    for (std::vector<Assignment *>::const_iterator I = m->global_variables.begin(),
             E = m->global_variables.end(); I != E; ++I) {
        used_names.insert((*I)->location->variable->name);
    }
}

Name ByReferencePass::get_unique_name() {
    std::string base = "_global_ref_" + as_string(unique_id++);
    Name name(base);
    unsigned i = 0;
    while (used_names.count(name)) {
        name = Name(base + "_" + as_string(i++));
    }
    return name;
}

void ByReferencePass::visit(Module *node) {
    initialize_unique_naming(node);

    for (std::vector<Function *>::const_iterator I = node->functions.begin(), E = node->functions.end(); I != E; ++I) {
        Function *f = *I;
        unsigned i = 0;
        for (std::vector<Variable *>::const_iterator AI = f->args.begin(), AE = f->args.end(); AI != AE; ++AI, ++i) {
            Variable *v = *AI;
            if (v->type().array()) {
                Name name = get_unique_name();
                Variable *gv = new Variable(name);
                gv->global = true;
                gv->set_type(v->type());
                v->set_reference(gv);
            }
        }
    }

    // Now visit as normal.
    for (std::vector<Assignment *>::const_iterator I = node->global_variables.begin(),
             E = node->global_variables.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    for (std::vector<Function *>::const_iterator I = node->functions.begin(),
             E = node->functions.end(); I != E; ++I) {
        (*I)->accept(this);
    }
}

void ByReferencePass::visit(FunctionCall *node) {
    Function *f = node->function;
    for (unsigned i = 0; i < node->args.size(); i++) {
        Assignment *a = node->args[i];
        if (f->args[i]->is_reference()) {
            delete a->location->variable;
            assert(a->location->offset == NULL);
            a->location->variable = f->args[i]->reference;
        }
        a->accept(this);
    }
}
