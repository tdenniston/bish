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
        block_print_braces = true;
        functioncall_wrap = false;
    }
    virtual void visit(const Module *);
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const ReturnStatement *);
    virtual void visit(const IfStatement *);
    virtual void visit(const ForLoop *);
    virtual void visit(const Function *);
    virtual void visit(const FunctionCall *);
    virtual void visit(const ExternCall *);
    virtual void visit(const Comparison *);
    virtual void visit(const Assignment *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
    virtual void visit(const Integer *);
    virtual void visit(const Fractional *);
    virtual void visit(const String *);
    virtual void visit(const Boolean *);
private:
    std::stack<LetScope *> let_stack;
    std::ostream &stream;
    unsigned indent_level;
    bool block_print_braces;
    bool functioncall_wrap;

    inline void disable_block_braces() { block_print_braces = false; }
    inline void enable_block_braces() { block_print_braces = true; }
    inline bool should_print_block_braces() const { return block_print_braces; }
    inline void disable_functioncall_wrap() { functioncall_wrap = false; }
    inline void enable_functioncall_wrap() { functioncall_wrap = true; }
    inline bool should_functioncall_wrap() const { return functioncall_wrap; }
    void indent();
    void push_let_scope(LetScope *s) { let_stack.push(s); }
    LetScope *pop_let_scope() { LetScope *s = let_stack.top(); let_stack.pop(); return s; }

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
