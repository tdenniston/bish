#ifndef __BISH_IR_VISITOR_H__
#define __BISH_IR_VISITOR_H__

namespace Bish {

class IRNode;
class Module;
class Block;
class Variable;
class ReturnStatement;
class ImportStatement;
class LoopControlStatement;
class IfStatement;
class ForLoop;
class Function;
class FunctionCall;
class ExternCall;
class IORedirection;
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
    virtual void visit(Module *);
    virtual void visit(Block *);
    virtual void visit(Variable *);
    virtual void visit(Function *);
    virtual void visit(FunctionCall *);
    virtual void visit(ExternCall *);
    virtual void visit(IORedirection *);
    virtual void visit(IfStatement *);
    virtual void visit(ImportStatement *);
    virtual void visit(ReturnStatement *);
    virtual void visit(LoopControlStatement *);
    virtual void visit(ForLoop *);
    virtual void visit(Assignment *);
    virtual void visit(BinOp *);
    virtual void visit(UnaryOp *);
    virtual void visit(Integer *);
    virtual void visit(Fractional *);
    virtual void visit(String *);
    virtual void visit(Boolean *);
};

}
#endif
