#ifndef __BISH_CODEGEN_BASH_H__
#define __BISH_CODEGEN_BASH_H__

#include <iostream>
#include "AST.h"
#include "ASTVisitor.h"

namespace Bish {
  
class CodeGen_Bash : public ASTVisitor {
public:
    CodeGen_Bash(std::ostream &os) : stream(os), indent_level(0) {}
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const IfStatement *);
    virtual void visit(const Function *);
    virtual void visit(const FunctionCall *);
    virtual void visit(const Comparison *);
    virtual void visit(const Assignment *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
    virtual void visit(const Integer *);
    virtual void visit(const Fractional *);
    virtual void visit(const String *);
    virtual void visit(const Boolean *);
private:
    std::ostream &stream;
    unsigned indent_level;
    bool block_print_braces;
    bool variable_print_dollar;

    inline void disable_block_braces() { block_print_braces = false; }
    inline void enable_block_braces() { block_print_braces = true; }
    inline bool should_print_block_braces() const { return indent_level > 0 && block_print_braces; }
    inline void disable_variable_dollar() { variable_print_dollar = false; }
    inline void enable_variable_dollar() { variable_print_dollar = true; }
    inline bool should_print_variable_dollar() const { return variable_print_dollar; }
};

}
#endif
