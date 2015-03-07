#ifndef __BISH_IR_VISITOR_H__
#define __BISH_IR_VISITOR_H__

namespace Bish {

class IRNode;
class Module;
class Block;
class Variable;
class IfStatement;
class ForLoop;
class Function;
class FunctionCall;
class ExternCall;
class Comparison;
class Assignment;
class BinOp;
class UnaryOp;
class Integer;
class Fractional;
class String;
class Boolean;

class IRVisitor {
public:
    virtual ~IRVisitor();
    virtual void visit(const Module *);
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const Function *);
    virtual void visit(const FunctionCall *);
    virtual void visit(const ExternCall *);
    virtual void visit(const IfStatement *);
    virtual void visit(const ForLoop *);
    virtual void visit(const Comparison *);
    virtual void visit(const Assignment *);
    virtual void visit(const BinOp *);
    virtual void visit(const UnaryOp *);
    virtual void visit(const Integer *);
    virtual void visit(const Fractional *);
    virtual void visit(const String *);
    virtual void visit(const Boolean *);
};

}
#endif
