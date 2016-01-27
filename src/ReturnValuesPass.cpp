#include "ReplaceIRNodes.h"
#include "ReturnValuesPass.h"

using namespace Bish;

namespace {

// Constructs an ordered list of all Block nodes in a function.
class GetAllBlocks : public IRVisitor {
public:
    GetAllBlocks(Function *f) {
        f->accept(this);
    }

    std::vector<Block *> blocks() { return block_vec; }

    virtual void visit(Block *b) {
        block_vec.push_back(b);
        IRVisitor::visit(b);
    }
private:
    std::vector<Block *> block_vec;
};

// Constructs an ordered list of all FunctionCall nodes in a statement
// IRNode. Any Blocks encountered are not recursively visited.
class GetAllCalls : public IRVisitor {
public:
    GetAllCalls(IRNode *stmt) {
        stmt->accept(this);
    }

    std::vector<FunctionCall *> calls() { return call_vec; }

    virtual void visit(Block *b) {
        // Do nothing.
    }

    virtual void visit(FunctionCall *call) {
        call_vec.push_back(call);
        IRVisitor::visit(call);
    }
private:
    std::vector<FunctionCall *> call_vec;
};

} // end anonymous namespace

void ReturnValuesPass::initialize_unique_naming(Module *m) {
    unique_id = 0;
    for (std::vector<Assignment *>::const_iterator I = m->global_variables.begin(),
             E = m->global_variables.end(); I != E; ++I) {
        used_names.insert((*I)->location->variable->name);
    }
}

Name ReturnValuesPass::get_unique_name(const std::string &prefix) {
    std::string base = prefix + as_string(unique_id++);
    Name name(base);
    unsigned i = 0;
    while (used_names.count(name)) {
        name = Name(base + "_" + as_string(i++));
    }
    return name;
}

void ReturnValuesPass::initialize_blacklist(Module *m) {
    class GetAllIORedirections : public IRVisitor {
    public:
        std::vector<IORedirection *> iors;
        virtual void visit(IORedirection *ior) {
            iors.push_back(ior);
            IRVisitor::visit(ior);
        }
    };
    GetAllIORedirections get;
    m->accept(&get);
    for (std::vector<IORedirection *>::iterator I = get.iors.begin(), E = get.iors.end(); I != E; ++I) {
        GetAllCalls get_calls(*I);
        std::vector<FunctionCall *> calls = get_calls.calls();
        for (std::vector<FunctionCall *>::iterator FI = calls.begin(), FE = calls.end(); FI != FE; ++FI) {
            Function *call = (*FI)->function;
            blacklist.insert(call);
        }
    }
}

void ReturnValuesPass::visit(Module *node) {
    initialize_unique_naming(node);
    initialize_blacklist(node);
    //process_statements(node->global_variables);

    for (std::vector<Function *>::iterator I = node->functions.begin(), E = node->functions.end(); I != E; ++I) {
        Function *f = *I;
        lower_function(f);
    }
}

template <class T>
void ReturnValuesPass::process_statements(std::vector<T *> &stmts) {
    typename std::vector<T*>::iterator SI;
    for (SI = stmts.begin(); SI != stmts.end(); ++SI) {
        IRNode *stmt = dynamic_cast<IRNode*>(*SI);
        assert(stmt);
        GetAllCalls get_calls(stmt);
        std::vector<FunctionCall *> calls = get_calls.calls();
        std::map<FunctionCall *, Variable *> replace;
        // goal here: for each call in stmt S, move the call to
        // before S. then add a local variable after the call that
        // saves the global retval of the function. then replace
        // the functioncall in S with that local var.
        for (std::vector<FunctionCall *>::iterator I = calls.begin(), E = calls.end(); I != E; ++I) {
            Function *call = (*I)->function;
            assert(call);
            if (blacklist.count(call)) continue;
            Variable *retval = get_return_value(call);
            if (retval == NULL) continue;
            Variable *v = new Variable(get_unique_name());
            Location *loc = new Location(v);
            Assignment *a = new Assignment(loc, retval, IRDebugInfo());
            SI = stmts.insert(SI, a);
            SI = stmts.insert(SI, *I);
            SI++; SI++;
            replace[*I] = v; // replace the function call with the saved retval.
        }
        ReplaceIRNodes replace_calls(replace);
        stmt->accept(&replace_calls);
    }
}

void ReturnValuesPass::lower_function(Function *f) {
    GetAllBlocks get_blocks(f);
    std::vector<Block *> blocks = get_blocks.blocks();
    std::set<Block *> block_set;
    std::set<FunctionCall *> call_set;
    for (std::vector<Block *>::iterator BI = blocks.begin(), BE = blocks.end(); BI != BE; ++BI) {
        Block *b = *BI;
        for (std::vector<IRNode *>::iterator I = b->nodes.begin(), E = b->nodes.end(); I != E; ++I) {
            GetAllCalls get_calls(*I);
            std::vector<FunctionCall *> calls = get_calls.calls();
            call_set.insert(calls.begin(), calls.end());
        }
    }

    // Create return values.
    for (std::set<FunctionCall *>::iterator I = call_set.begin(), E = call_set.end(); I != E; ++I) {
        Function *call = (*I)->function;
        assert(call);
        if (blacklist.count((*I)->function)) continue;
        get_return_value(call);
        Block *parent = dynamic_cast<Block*>((*I)->parent());
        assert(parent);
        block_set.insert(parent);
    }

    // Now convert function calls to use the return value.
    for (std::set<Block *>::iterator BI = block_set.begin(), BE = block_set.end(); BI != BE; ++BI) {
        Block *b = *BI;
        process_statements(b->nodes);
    }
}

Variable *ReturnValuesPass::get_return_value(Function *f) {
    if (return_values.find(f) != return_values.end()) {
        return return_values[f];
    }
    bool ret_void = true;
    Variable *gv = new Variable(get_unique_name("_global_retval_"));
    gv->global = true;
    GetAllBlocks get_blocks(f);
    std::vector<Block *> blocks = get_blocks.blocks();
    for (std::vector<Block *>::iterator BI = blocks.begin(), BE = blocks.end(); BI != BE; ++BI) {
        Block *b = *BI;
        for (std::vector<IRNode *>::iterator SI = b->nodes.begin(); SI != b->nodes.end(); ++SI) {
            if (ReturnStatement *ret = dynamic_cast<ReturnStatement*>(*SI)) {
                ret_void = false;
                // Replace return statement with assignment to global variable
                Assignment *a = new Assignment(new Location(gv), ret->value, IRDebugInfo());
                SI = b->nodes.insert(SI, a);
                SI++;
                // Remove return value.
                ret->value = NULL;
            }
        }
    }
    if (ret_void) {
        delete gv;
        gv = NULL;
    }
    return_values[f] = gv;
    return gv;
}
