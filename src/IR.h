#ifndef __BISH_IR_H__
#define __BISH_IR_H__

#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include "IRVisitor.h"
#include "Util.h"
#include "Type.h"

namespace Bish {

// Encapsulates information required for emitting error messages
// related to IRNodes.
class IRDebugInfo {
public:
    // Name of source file
    std::string file;
    // Index into source text where the IRNode begins.
    unsigned start;
    // Index into source text where the IRNode ends.
    unsigned end;
    // Line number in source text where IRNode resides.
    unsigned lineno;
    IRDebugInfo() : file(""), start(0), end(0), lineno(0) {}
    IRDebugInfo(const std::string &f, unsigned s, unsigned e, unsigned lno) :
        file(f), start(s), end(e), lineno(lno) {}
    IRDebugInfo(const IRDebugInfo &a) :
        file(a.file), start(a.start), end(a.end), lineno(a.lineno) {}
};

class IRNode {
public:
    IRNode() : type_(UndefinedTy), parent_(NULL) {}
    IRNode(const IRDebugInfo &info) : type_(UndefinedTy), parent_(NULL), debug_info_(info) {}
    virtual ~IRNode() {}
    virtual void accept(IRVisitor *v) = 0;
    Type type() const { return type_; }
    void set_type(Type t) { type_ = t; }
    IRNode *parent() const { return parent_; }
    void set_parent(IRNode *p) { parent_ = p; }
protected:
    Type type_;
    IRNode *parent_;
    IRDebugInfo debug_info_;
};

// This is the "curiously recurring template" pattern. It's used to
// avoid having to implement the 'accept' method in every derived
// class.
template<typename T>
class BaseIRNode : public IRNode {
public:
    BaseIRNode() {}
    BaseIRNode(const IRDebugInfo &info) : IRNode(info) {}
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
    iterator begin() { return nodes.begin(); }
    iterator end() { return nodes.end(); }
};

// The name of a symbol, with optional namespace qualifier(s).
class Name {
public:
    std::vector<std::string> namespace_id;
    std::string name;
    Name(const std::string &n) : name(n) {}
    Name(const std::string &n, const std::string &ns) : name(n) {
        namespace_id.push_back(ns);
    }
    Name(const Name &n) : name(n.name) {
        namespace_id.insert(namespace_id.begin(), n.namespace_id.begin(), n.namespace_id.end());
    }

    std::string str(const char sep='_') const {
        std::string result;
        for (std::vector<std::string>::const_iterator I = namespace_id.begin(),
                 E = namespace_id.end(); I != E; ++I) {
            result += *I + sep;
        }
        result += name;
        return result;
    }

    void add_namespace(const std::string &ns) {
        namespace_id.insert(namespace_id.begin(), ns);
    }

    bool namespaces_equal(const Name &b) const {
        return namespace_id == b.namespace_id;
    }

    bool has_namespace(const std::string &ns) const {
        for (std::vector<std::string>::const_iterator I = namespace_id.begin(),
                 E = namespace_id.end(); I != E; ++I) {
            if (*I == ns) return true;
        }
        return false;
    }

    // Define lexicographic sort so this may be a key for std::map.
    bool operator<(const Name &b) const {
        return (namespace_id < b.namespace_id ||
                (namespace_id == b.namespace_id && name < b.name));
    }

    bool operator==(const Name &b) const {
        return namespace_id == b.namespace_id && name == b.name;
    }
};

class Variable : public BaseIRNode<Variable> {
public:
    Name name;
    bool global;
    Variable(const Name &n) : name(n), global(false) {}
};

class Location : public BaseIRNode<Location> {
public:
    Variable *variable;
    IRNode *offset;
    Location(Variable *v) : variable(v), offset(NULL) {}
    Location(Variable *v, IRNode *o) : variable(v), offset(o) {}

    bool is_array_ref() const { return offset != NULL; }
    bool is_variable() const { return offset == NULL; }
};

class Function : public BaseIRNode<Function> {
public:
    Name name;
    std::vector<Variable *> args;
    Block *body;

    Function(const Name &n) : name(n) {
        body = NULL;
    }

    Function(const Name &n, Block *b) : name(n) {
        body = b;
    }

    Function(const Name &n, const std::vector<Variable *> &a, Block *b) : name(n) {
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
    // List of all functions in the module (including main)
    std::vector<Function *> functions;
    // List of all global variables.
    std::vector<Assignment *> global_variables;
    // Pointer to main function
    Function *main;
    // Path to source file on disk
    std::string path;
    // Namespace identifier
    std::string namespace_id;

    Module() : main(NULL) {}

    // Set the module's main function.
    void set_main(Function *f);
    // Add the given function to this module.
    void add_function(Function *f);
    // Add the given global variable assignment.
    void add_global(Assignment *a);
    // Set the module's path on disk and corresponding namespace.
    void set_path(const std::string &p);
    // Return the function in this module corresponding to the given
    // name, or NULL if no such function exists.
    Function *get_function(const Name &name) const;
    // Import functions from the given module if they are called from
    // this module.
    void import(Module *m);
};

class Assignment : public BaseIRNode<Assignment> {
public:
    Location *location;
    std::vector<IRNode *> values;
    Assignment(Location *loc, const std::vector<IRNode *> &val, const IRDebugInfo &info) :
        location(loc), values(val.begin(), val.end()), BaseIRNode(info) {}
};

class ImportStatement : public BaseIRNode<ImportStatement> {
public:
    std::string module_name;
    std::string path;
    ImportStatement(const Module *m, const std::string &qual_name, const IRDebugInfo &info) : BaseIRNode(info) {
        std::string path_ = dirname(m->path) + "/" + qual_name + ".bish";
        path = abspath(path_);
        assert(!path.empty() && "Could not resolve module path from import.");
        module_name = module_name_from_path(qual_name);
        assert(!module_name.empty());
    }
};

class ReturnStatement : public BaseIRNode<ReturnStatement> {
public:
    IRNode *value;
    ReturnStatement(IRNode *v, const IRDebugInfo &info) : value(v), BaseIRNode(info) {}
};

class LoopControlStatement : public BaseIRNode<LoopControlStatement> {
public:
    typedef enum { Break, Continue } Operator;
    Operator op;
    LoopControlStatement(Operator op_, const IRDebugInfo &info) : op(op_), BaseIRNode(info) {}
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
    FunctionCall(Function *f, const IRDebugInfo &info) : BaseIRNode(info) {
        function = f;
    }
    FunctionCall(Function *f, const std::vector<IRNode *> &a, const IRDebugInfo &info) : BaseIRNode(info) {
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
    ExternCall(InterpolatedString *b, const IRDebugInfo &info) : body(b), BaseIRNode(info) {}
};

class IORedirection : public BaseIRNode<IORedirection> {
public:
    typedef enum { Pipe } Operator;
    Operator op;
    IRNode *a, *b;
    IORedirection(Operator op_, IRNode *a_, IRNode *b_, const IRDebugInfo &info) : op(op_), a(a_), b(b_), BaseIRNode(info) {}
};

class BinOp : public BaseIRNode<BinOp> {
public:
    typedef enum { Add, Sub, Mul, Div, Mod, Eq, NotEq, LT, LTE, GT, GTE, And, Or } Operator;
    Operator op;
    IRNode *a, *b;
    BinOp(Operator op_, IRNode *a_, IRNode *b_, const IRDebugInfo &info) : op(op_), a(a_), b(b_), BaseIRNode(info) {}
};

class UnaryOp : public BaseIRNode<UnaryOp> {
public:
    typedef enum { Negate, Not } Operator;
    Operator op;
    IRNode *a;
    UnaryOp(Operator op_, IRNode *a_, const IRDebugInfo &info) : op(op_), a(a_), BaseIRNode(info) {}
};

class Integer : public BaseIRNode<Integer> {
public:
    int value;
    Integer(const std::string &s) : value(convert_string<int>(s)) {}
};

class Fractional : public BaseIRNode<Fractional> {
public:
    double value;
    Fractional(const std::string &s) : value(convert_string<double>(s)) {}
};

class String : public BaseIRNode<String> {
public:
    InterpolatedString *value;
    String(InterpolatedString *s) : value(s) {}
};

class Boolean : public BaseIRNode<Boolean> {
public:
    bool value;
    Boolean(bool v) : value(v) {}
};

// Return the Bish Type to represent the given IR node.
Type get_primitive_type(const IRNode *n);

}
#endif
