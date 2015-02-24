#ifndef __BISH_AST_H__
#define __BISH_AST_H__

#include <sstream>
#include <string>
#include <vector>

namespace {
template <typename T>
T convert_string(const std::string &s) {
    T t;
    std::istringstream(s) >> t;
    return t;
}
};

namespace Bish {

class ASTNode {

};

class Block : public ASTNode {
public:
    std::vector<ASTNode *> nodes;
    Block(const std::vector<ASTNode *> &n) {
        nodes.insert(nodes.begin(), n.begin(), n.end());
    }
};

class Variable : public ASTNode {
public:
    std::string name;
    Variable(const std::string &n) : name(n) {}
};

class Assignment : public ASTNode {
public:
    Variable *variable;
    ASTNode *value;
    Assignment(Variable *var, ASTNode *val) : variable(var), value(val) {}
};

class BinOp : public ASTNode {
public:
    typedef enum { Add, Sub, Mul, Div } Operator;
    Operator op;
    ASTNode *a, *b;
    BinOp(Operator op_, ASTNode *a_, ASTNode *b_) : op(op_), a(a_), b(b_) {}
};

class UnaryOp : public ASTNode {
public:
    typedef enum { Negate } Operator;
    Operator op;
    ASTNode *a;
    UnaryOp(Operator op_, ASTNode *a_) : op(op_), a(a_) {}
};

class Integer : public ASTNode {
public:
    int value;
    Integer(int i) : value(i) {}
    Integer(const std::string &s) : value(convert_string<int>(s)) {}
};

class Fractional : public ASTNode {
public:
    double value;
    Fractional(double d) : value(d) {}
    Fractional(const std::string &s) : value(convert_string<double>(s)) {}
};

class String : public ASTNode {
public:
    std::string value;
    String(const std::string &s) : value(s) {}
};

class Boolean : public ASTNode {
public:
    bool value;
    Boolean(bool v) : value(v) {}
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
