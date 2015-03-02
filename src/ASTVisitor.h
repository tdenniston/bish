#ifndef __BISH_AST_VISITOR_H__
#define __BISH_AST_VISITOR_H__

namespace Bish {

class ASTNode;
class Block;
class Variable;
class NodeList;
class IfStatement;
class Function;
class FunctionCall;
class Comparison;
class Assignment;
class BinOp;
class UnaryOp;
class Integer;
class Fractional;
class String;
class Boolean;

class ASTVisitor {
public:
    virtual ~ASTVisitor();
    virtual void visit(const Block *);
    virtual void visit(const Variable *);
    virtual void visit(const NodeList *);
    virtual void visit(const Function *);
    virtual void visit(const FunctionCall *);
    virtual void visit(const IfStatement *);
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
