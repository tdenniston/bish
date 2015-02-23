#ifndef __BISH_AST_H__
#define __BISH_AST_H__

#include <string>
#include <vector>

namespace Bish {

class ASTNode {
public:
    ASTNode() : parent_(NULL) {}
    ASTNode(ASTNode *p) : parent_(p) {}
    ASTNode *root() const;
    ASTNode *parent() const;
private:
    ASTNode *parent_;
};

class Block : public ASTNode {
public:
    Block(ASTNode *p) : ASTNode(p) {}
    std::vector<ASTNode *> nodes;
};

class Variable : public ASTNode {
public:
    std::string name;
    Variable(ASTNode *p, const std::string &n) : ASTNode(p), name(n) {}
};

class Assignment : public ASTNode {
public:
    Variable *variable;
    ASTNode *value;
    Assignment(ASTNode *p, Variable *var, ASTNode *val) : ASTNode(p), variable(var), value(val) {}
};

class BinOp : public ASTNode {
public:
    typedef enum { Add, Sub, Mul, Div } Operator;
    Operator op;
    ASTNode *a, *b;
    BinOp(ASTNode *p, Operator op_, ASTNode *a_, ASTNode *b_) : ASTNode(p), op(op_), a(a_), b(b_) {}
};

class UnaryOp : public ASTNode {
public:
    typedef enum { Negate } Operator;
    Operator op;
    ASTNode *a;
    UnaryOp(ASTNode *p, Operator op_, ASTNode *a_) : ASTNode(p), op(op_), a(a_) {}
};

class Integer : public ASTNode {
public:
    int value;
    Integer(ASTNode *p, int i) : ASTNode(p), value(i) {}
};

class Fractional : public ASTNode {
public:
    double value;
    Fractional(ASTNode *p, double d) : ASTNode(p), value(d) {}
};

class String : public ASTNode {
public:
    std::string value;
    String(ASTNode *p, const std::string &s) : ASTNode(p), value(s) {}
};
 
class AST {
public:
    AST() : root(NULL) {}
    AST(ASTNode *r) : root(r) {}
    
private:
    ASTNode *root;
};

}
#endif
