#ifndef __BISH_RETURN_VALUES_PASS_H__
#define __BISH_RETURN_VALUES_PASS_H__

#include "IR.h"
#include "IRVisitor.h"
#include <map>

namespace Bish {

class ReturnValuesPass : public IRVisitor {
public:
    virtual void visit(Module *);
private:
    unsigned unique_id;
    std::set<Function *> blacklist;
    std::set<Name> used_names;
    std::map<Function *, std::map<unsigned, Variable *> > reference_vars;
    std::map<Function *, Variable *> return_values;
    Name get_unique_name(const std::string &prefix="_rv_");
    void initialize_unique_naming(Module *m);
    void initialize_blacklist(Module *m);
    void lower_function(Function *f);
    Variable *get_return_value(Function *f);
    template <class T> void process_statements(std::vector<T *> &stmts);
};

}

#endif
