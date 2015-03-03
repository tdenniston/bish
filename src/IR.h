#ifndef __BISH_IR_H__
#define __BISH_IR_H__

#include <sstream>
#include <string>
#include <vector>
#include "IRVisitor.h"
#include "Type.h"

namespace {
template <typename T>
T convert_string(const std::string &s) {
    T t;
    std::istringstream(s) >> t;
    return t;
}
};

namespace Bish {

class IRNode {
public:
    IRNode() : type_(UndefinedTy), parent_(NULL) {}
    virtual ~IRNode() {}
    virtual void accept(IRVisitor *v) const = 0;
    Type type() const { return type_; }
    IRNode *parent() const { return parent_; }
protected:
    Type type_;
    IRNode *parent_;
};

// This is the "curiously recurring template" pattern. It's used to
// avoid having to implement the 'accept' method in every derived
// class.
template<typename T>
class BaseIRNode : public IRNode {
public:
    void accept(IRVisitor *v) const {
        v->visit((const T *)this);
    }
};

class Block : public BaseIRNode<Block> {
public:
    typedef std::vector<IRNode *>::iterator iterator;
    std::vector<IRNode *> nodes;
    Block(const std::vector<IRNode *> &n) {
        nodes.insert(nodes.begin(), n.begin(), n.end());
    }
};

class Variable : public BaseIRNode<Variable> {
public:
    std::string name;
    Variable(const std::string &n) : name(n) {}
};

class Function : public BaseIRNode<Function> {
public:
    std::string name;
    std::vector<Variable *> args;
    Block *body;

    Function(const std::string &n, Block *b) {
        name = n;
        body = b;
    }

    Function(const std::string &n, const std::vector<Variable *> &a, Block *b) {
        name = n;
        args.insert(args.begin(), a.begin(), a.end());
        body = b;
    }
};

class Module : public BaseIRNode<Module> {
public:
    std::vector<Function *> functions;
    Function *main;
    Module() : main(NULL) {}
    Module(const std::vector<Function *> &f, Function *m) {
        functions.insert(functions.begin(), f.begin(), f.end());
        functions.push_back(m);
        main = m;
    }

    void set_main(Function *f);
    void add_function(Function *f);
};

class Assignment : public BaseIRNode<Assignment> {
public:
    Variable *variable;
    IRNode *value;
    Assignment(Variable *var, IRNode *val) : variable(var), value(val) {}
};

class IfStatement : public BaseIRNode<IfStatement> {
public:
    IRNode *condition;
    IRNode *body;
    IfStatement(IRNode *c, IRNode *b) : condition(c), body(b) {}
};

class FunctionCall : public BaseIRNode<FunctionCall> {
public:
    std::string name;
    std::vector<IRNode *> args;
    FunctionCall(const std::string &n, const std::vector<IRNode *> &a) {
        name = n;
        args.insert(args.begin(), a.begin(), a.end());
    }
};

class Comparison : public BaseIRNode<Comparison> {
public:
    IRNode *a;
    IRNode *b;
    Comparison(IRNode *a_, IRNode *b_) : a(a_), b(b_) {}
};
 
class BinOp : public BaseIRNode<BinOp> {
public:
    typedef enum { Add, Sub, Mul, Div } Operator;
    Operator op;
    IRNode *a, *b;
    BinOp(Operator op_, IRNode *a_, IRNode *b_) : op(op_), a(a_), b(b_) {}
};

class UnaryOp : public BaseIRNode<UnaryOp> {
public:
    typedef enum { Negate } Operator;
    Operator op;
    IRNode *a;
    UnaryOp(Operator op_, IRNode *a_) : op(op_), a(a_) {}
};

class Integer : public BaseIRNode<Integer> {
public:
    int value;
    Integer(int i) : value(i) {}
    Integer(const std::string &s) : value(convert_string<int>(s)) {}
};

class Fractional : public BaseIRNode<Fractional> {
public:
    double value;
    Fractional(double d) : value(d) {}
    Fractional(const std::string &s) : value(convert_string<double>(s)) {}
};

class String : public BaseIRNode<String> {
public:
    std::string value;
    String(const std::string &s) : value(s) {}
};

class Boolean : public BaseIRNode<Boolean> {
public:
    bool value;
    Boolean(bool v) : value(v) {}
};

}
#endif
