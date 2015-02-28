#ifndef __BISH_AST_H__
#define __BISH_AST_H__

#include <sstream>
#include <string>
#include <vector>
#include "ASTVisitor.h"
#include "SymbolTable.h"

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
public:
    virtual ~ASTNode() {}
    virtual void accept(ASTVisitor *v) const = 0;
};

// This is the "curiously recurring template" pattern. It's used to
// avoid having to implement the 'accept' method in every derived
// class.
template<typename T>
class BaseASTNode : public ASTNode {
public:
    void accept(ASTVisitor *v) const {
        v->visit((const T *)this);
    }
};

class Block : public BaseASTNode<Block> {
public:
    SymbolTable *symbol_table;
    std::vector<ASTNode *> nodes;
    Block(const std::vector<ASTNode *> &n) {
        nodes.insert(nodes.begin(), n.begin(), n.end());
        symbol_table = new SymbolTable();
    }
};

class Variable : public BaseASTNode<Variable> {
public:
    std::string name;
    Variable(const std::string &n) : name(n) {}
};

class Assignment : public BaseASTNode<Assignment> {
public:
    Variable *variable;
    ASTNode *value;
    Assignment(Variable *var, ASTNode *val) : variable(var), value(val) {}
};

class BinOp : public BaseASTNode<BinOp> {
public:
    typedef enum { Add, Sub, Mul, Div } Operator;
    Operator op;
    ASTNode *a, *b;
    BinOp(Operator op_, ASTNode *a_, ASTNode *b_) : op(op_), a(a_), b(b_) {}
};

class UnaryOp : public BaseASTNode<UnaryOp> {
public:
    typedef enum { Negate } Operator;
    Operator op;
    ASTNode *a;
    UnaryOp(Operator op_, ASTNode *a_) : op(op_), a(a_) {}
};

class Integer : public BaseASTNode<Integer> {
public:
    int value;
    Integer(int i) : value(i) {}
    Integer(const std::string &s) : value(convert_string<int>(s)) {}
};

class Fractional : public BaseASTNode<Fractional> {
public:
    double value;
    Fractional(double d) : value(d) {}
    Fractional(const std::string &s) : value(convert_string<double>(s)) {}
};

class String : public BaseASTNode<String> {
public:
    std::string value;
    String(const std::string &s) : value(s) {}
};

class Boolean : public BaseASTNode<Boolean> {
public:
    bool value;
    Boolean(bool v) : value(v) {}
};

class AST {
public:
    AST() : root_(NULL) {}
    AST(ASTNode *r) : root_(r) {}
    ASTNode *root() { return root_; }
    void accept(ASTVisitor *v) const { root_->accept(v); }
private:
    ASTNode *root_;
};

class ASTPrinter {
public:
    void print(AST *ast);
protected:
    virtual void print_block(Block *n, std::ostream &os) {}
    virtual void print_variable(Variable *n, std::ostream &os) {}
    virtual void print_assignment(Assignment *n, std::ostream &os) {}
    virtual void print_binop(BinOp *n, std::ostream &os) {}
    virtual void print_unaryop(UnaryOp *n, std::ostream &os) {}
    virtual void print_integer(Integer *n, std::ostream &os) {}
    virtual void print_fractional(Fractional *n, std::ostream &os) {}
    virtual void print_string(String *n, std::ostream &os) {}
    virtual void print_boolean(Boolean *n, std::ostream &os) {}
    void print(ASTNode *n, std::ostream &os);
};

class BishPrinter : public ASTPrinter {
protected:
    virtual void print_block(Block *n, std::ostream &os);
    virtual void print_variable(Variable *n, std::ostream &os);
    virtual void print_assignment(Assignment *n, std::ostream &os);
    virtual void print_binop(BinOp *n, std::ostream &os);
    virtual void print_unaryop(UnaryOp *n, std::ostream &os);
    virtual void print_integer(Integer *n, std::ostream &os);
    virtual void print_fractional(Fractional *n, std::ostream &os);
    virtual void print_string(String *n, std::ostream &os);
    virtual void print_boolean(Boolean *n, std::ostream &os);
};

}
#endif
