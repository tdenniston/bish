#include <iostream>
#include <cassert>
#include "IR.h"
#include "TypeChecker.h"

using namespace Bish;

void TypeChecker::visit(ReturnStatement *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    node->value->accept(this);
    node->set_type(node->value->type());
    // Propagate type of this return statement to the parent function.
    Function *f = dynamic_cast<Function*>(node->parent()->parent());
    assert(f);
    if (f->type() != UndefinedTy) {
        assert(f->type() == node->value->type() && "Invalid return type for function.");
    }
    propagate_if_undef(f, node);
}

void TypeChecker::visit(Function *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    for (std::vector<Variable *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I) {
        (*I)->accept(this);
    }
    if (node->body) node->body->accept(this);
}

void TypeChecker::visit(FunctionCall *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    node->function->accept(this);
    for (std::vector<IRNode *>::const_iterator I = node->args.begin(),
           E = node->args.end(); I != E; ++I) {
      (*I)->accept(this);
    }
    node->set_type(node->function->type());
}

void TypeChecker::visit(ExternCall *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    node->set_type(UndefinedTy);
}

void TypeChecker::visit(IORedirection *node) {
    if (node->type() != UndefinedTy) return;
    node->a->accept(this);
    node->b->accept(this);
    node->set_type(UndefinedTy);
}

void TypeChecker::visit(Assignment *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    node->variable->accept(this);
    node->value->accept(this);
    if (node->variable->type() != UndefinedTy && node->value->type() != UndefinedTy) {
        assert(node->variable->type() == node->value->type() &&
               "Invalid type in assignment.");
    } else {
        node->variable->set_type(node->value->type());
    }
    node->set_type(node->variable->type());
}

void TypeChecker::visit(BinOp *node) {
    if (visited(node) || node->type() != UndefinedTy) return;
    visited_set.insert(node);
    node->a->accept(this);
    node->b->accept(this);
    propagate_if_undef(node->a, node->b);
    assert(node->a->type() == node->b->type());
    switch (node->op) {
    case BinOp::Eq:
    case BinOp::NotEq:
    case BinOp::LT:
    case BinOp::LTE:
    case BinOp::GT:
    case BinOp::GTE:
    case BinOp::And:
    case BinOp::Or:
        node->set_type(BooleanTy);
        break;
    case BinOp::Add:
    case BinOp::Sub:
    case BinOp::Mul:
    case BinOp::Div:
    case BinOp::Mod:
        node->set_type(node->a->type());
        break;
    }
}

void TypeChecker::visit(UnaryOp *node) {
    if (visited(node)) return;
    visited_set.insert(node);
    node->a->accept(this);
    node->set_type(node->a->type());
}

void TypeChecker::visit(Integer *node) {
    node->set_type(IntegerTy);
}

void TypeChecker::visit(Fractional *node) {
    node->set_type(FractionalTy);
}

void TypeChecker::visit(String *node) {
    node->set_type(StringTy);
}

void TypeChecker::visit(Boolean *node) {
    node->set_type(BooleanTy);
}

void TypeChecker::propagate_if_undef(IRNode *a, IRNode *b) {
    if (a->type() == UndefinedTy) {
        a->set_type(b->type());
    } else if (b->type() == UndefinedTy) {
        b->set_type(a->type());
    }
}
