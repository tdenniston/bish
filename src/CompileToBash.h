#ifndef __BISH_COMPILE_TO_BASH_H__
#define __BISH_COMPILE_TO_BASH_H__

#include <iostream>
#include "AST.h"
#include "ASTVisitor.h"

namespace Bish {
  
class CompileToBash : public ASTVisitor {
public:
    CompileToBash(std::ostream &os) : stream(os), indent_level(0) {}
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const IfStatement *);
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
};

}
#endif
