#ifndef __BISH_IR_H__
#define __BISH_IR_H__

#include <sstream>
#include <string>
#include <vector>
#include "IRVisitor.h"
#include "Util.h"
#include "Type.h"

namespace Bish {

class IRNode {
public:
    IRNode() : type_(UndefinedTy), parent_(NULL) {}
    virtual ~IRNode() {}
    virtual void accept(IRVisitor *v) = 0;
    Type type() const { return type_; }
    void set_type(Type t) { type_ = t; }
    IRNode *parent() const { return parent_; }
    void set_parent(IRNode *p) { parent_ = p; }
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
    void accept(IRVisitor *v) {
        v->visit((T *)this);
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

    Function(const std::string &n) {
        name = n;
        body = NULL;
    }

    Function(const std::string &n, Block *b) {
        name = n;
        body = b;
    }

    Function(const std::string &n, const std::vector<Variable *> &a, Block *b) {
        name = n;
        args.insert(args.begin(), a.begin(), a.end());
        body = b;
    }

    void set_args(const std::vector<Variable *> &a) {
        args.clear();
        args.insert(args.begin(), a.begin(), a.end());
    }

    void set_body(Block *b) {
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
    Function *get_function(const std::string &name) const {
        for (std::vector<Function *>::const_iterator I = functions.begin(),
                 E = functions.end(); I != E; ++I) {
            if (name.compare((*I)->name) == 0) {
                return *I;
            }
        }
        return NULL;
    }
};

class Assignment : public BaseIRNode<Assignment> {
public:
    Variable *variable;
    IRNode *value;
    Assignment(Variable *var, IRNode *val) : variable(var), value(val) {}
};

class ReturnStatement : public BaseIRNode<ReturnStatement> {
public:
    IRNode *value;
    ReturnStatement(IRNode *v) : value(v) {}
};

// Helper class for IfStatement
class PredicatedBlock {
public:
    IRNode *condition;
    IRNode *body;
    PredicatedBlock(IRNode *c, IRNode *b) : condition(c), body(b) {}
    PredicatedBlock(PredicatedBlock *p) {
        condition = p->condition;
        body = p->body;
    }
};

class IfStatement : public BaseIRNode<IfStatement> {
public:
    PredicatedBlock *pblock;
    std::vector<PredicatedBlock *> elses;
    IRNode *elseblock;
    IfStatement(IRNode *c, IRNode *b) {
        pblock = new PredicatedBlock(c, b);
        elseblock = NULL;
    }

    IfStatement(IRNode *c, IRNode *b, IRNode *e) {
        pblock = new PredicatedBlock(c, b);
        elseblock = e;
    }

    IfStatement(IRNode *c, IRNode *b, const std::vector<PredicatedBlock *> &es, IRNode *e) {
        pblock = new PredicatedBlock(c, b);
        elseblock = e;
        elses.insert(elses.begin(), es.begin(), es.end());
    }
};

class ForLoop : public BaseIRNode<ForLoop> {
public:
    Variable *variable;
    IRNode *lower, *upper;
    IRNode *body;
    ForLoop(Variable *v, IRNode *l, IRNode *u, IRNode *b) :
        variable(v), lower(l), upper(u), body(b) {}
};

class FunctionCall : public BaseIRNode<FunctionCall> {
public:
    Function *function;
    std::vector<IRNode *> args;
    FunctionCall(Function *f) {
        function = f;
    }
    FunctionCall(Function *f, const std::vector<IRNode *> &a) {
        function = f;
        args.insert(args.begin(), a.begin(), a.end());
    }
};

// Helper class to represent interpolated strings.
class InterpolatedString {
public:
    class Item {
    public:
        Item(const std::string &s) : str_(s), var_(NULL), ty(STR) {}
        Item(Variable *v) : var_(v), ty(VAR) {}
        bool is_str() const { return ty == STR; }
        bool is_var() const { return ty == VAR; }
        const std::string &str() const { return str_; }
        Variable *var() const { return var_; }
    private:
        typedef enum { STR, VAR } Type;
        Type ty;
        std::string str_;
        Variable *var_;
    };

    void push_str(const std::string &s) {
        items.push_back(Item(s));
    }

    void push_var(Variable *v) {
        items.push_back(Item(v));
    }

    std::string interpolate() {
        return "";
    }

    typedef std::vector<Item>::const_iterator const_iterator;
    const_iterator begin() { return items.begin(); }
    const_iterator end() { return items.end(); }
private:
    std::string body;
    std::vector<Variable *> to_interpolate;
    std::vector<Item> items;
};

class ExternCall : public BaseIRNode<ExternCall> {
public:
    InterpolatedString *body;
    ExternCall(InterpolatedString *b) : body(b) {}
};

class BinOp : public BaseIRNode<BinOp> {
public:
    typedef enum { Add, Sub, Mul, Div, Mod, Eq, NotEq, LT, LTE, GT, GTE } Operator;
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
