#ifndef __BISH_CODEGEN_BASH_H__
#define __BISH_CODEGEN_BASH_H__

#include <stack>
#include <map>
#include <iostream>
#include "IR.h"
#include "IRVisitor.h"

namespace Bish {

class LetScope {
public:
    void add(const Variable *v, const std::string &newname) {
        rename[v] = newname;
    }

    bool lookup(const Variable *v, std::string &out) {
        std::map<const Variable *, std::string>::iterator I = rename.find(v);
        if (I != rename.end()) {
            out = I->second;
            return true;
        } else {
            return false;
        }
    }
private:
    std::map<const Variable *, std::string> rename;
};
 
class CodeGen_Bash : public IRVisitor {
public:
    CodeGen_Bash(std::ostream &os) : stream(os) {
        indent_level = 0;
        enable_block_braces();
        disable_functioncall_wrap();
        enable_quote_variable();
        enable_comparison_wrap();
    }
    virtual void visit(Module *);
    virtual void visit(Block *);
    virtual void visit(Variable *);
    virtual void visit(ReturnStatement *);
    virtual void visit(LoopControlStatement *);
    virtual void visit(IfStatement *);
    virtual void visit(ForLoop *);
    virtual void visit(Function *);
    virtual void visit(FunctionCall *);
    virtual void visit(ExternCall *);
    virtual void visit(IORedirection *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
    virtual void visit(Integer *);
    virtual void visit(Fractional *);
    virtual void visit(String *);
    virtual void visit(Boolean *);
private:
    std::stack<LetScope *> let_stack;
    std::stack<Function *> function_args_insert;
    std::stack<bool> block_print_braces;
    std::stack<bool> functioncall_wrap;
    std::stack<bool> quote_variable;
    std::stack<bool> comparison_wrap;
    std::ostream &stream;
    unsigned indent_level;

    inline void disable_block_braces() { block_print_braces.push(false); }
    inline void enable_block_braces() { block_print_braces.push(true); }
    inline void reset_block_braces() { block_print_braces.pop(); }
    inline bool should_print_block_braces() const { return block_print_braces.top(); }

    inline void disable_functioncall_wrap() { functioncall_wrap.push(false); }
    inline void enable_functioncall_wrap() { functioncall_wrap.push(true); }
    inline void reset_functioncall_wrap() { functioncall_wrap.pop(); }
    inline bool should_functioncall_wrap() const { return functioncall_wrap.top(); }

    inline void disable_quote_variable() { quote_variable.push(false); }
    inline void enable_quote_variable() { quote_variable.push(true); }
    inline void reset_quote_variable() { quote_variable.pop(); }
    inline bool should_quote_variable() const { return quote_variable.top(); }

    inline void disable_comparison_wrap() { comparison_wrap.push(false); }
    inline void enable_comparison_wrap() { comparison_wrap.push(true); }
    inline void reset_comparison_wrap() { comparison_wrap.pop(); }
    inline bool should_comparison_wrap() const { return comparison_wrap.top(); }

    inline bool should_emit_statement(const IRNode *node) const;

    bool is_equals_op(IRNode *n) const {
        if (BinOp *b = dynamic_cast<BinOp*>(n)) {
            return b->op == BinOp::Eq;
        }
        return false;
    }

    void indent();
    void push_let_scope(LetScope *s) { let_stack.push(s); }
    LetScope *pop_let_scope() { LetScope *s = let_stack.top(); let_stack.pop(); return s; }
    void push_function_args_insert(Function *f) { function_args_insert.push(f); }
    void pop_function_args_insert() { function_args_insert.pop(); }

    bool lookup_let(const Variable *v, std::string &out) {
        std::stack<LetScope *> aux;
        std::string tmp;
        bool found = false;
        while (!let_stack.empty()) {
            if (let_stack.top()->lookup(v, tmp)) {
                out = tmp;
                found = true;
                break;
            }
            aux.push(let_stack.top());
            let_stack.pop();
        }
        while (!aux.empty()) {
            let_stack.push(aux.top());
            aux.pop();
        }
        return found;
    }

    std::string lookup_name(const Variable *v) {
        std::string tmp;
        if (lookup_let(v, tmp)) {
            return tmp;
        } else {
            return v->name;
        }
    }
};

}
#endif
