#include <iostream>
#include <cassert>
#include "Errors.h"
#include "IR.h"
#include "TypeChecker.h"

using namespace Bish;

void TypeChecker::visit(Module *node) {
    if (visited(node)) return;
    visited_set.insert(node);
    module = node;
    node->global_variables->accept(this);
    // Functions are not visited here: only via function calls.
    if (node->main) node->main->accept(this);
}

void TypeChecker::visit(Location *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    if (node->is_array_ref()) {
        bish_assert(node->variable->type().array()) <<
            "Invalid use of array reference on non-array variable";
        node->set_type(node->variable->type().element());
    } else {
        node->set_type(node->variable->type());
    }
}

void TypeChecker::visit(ReturnStatement *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    if (node->value == NULL) return;
    node->value->accept(this);
    node->set_type(node->value->type());
    // Propagate type of this return statement to the parent function.
    Function *f = dynamic_cast<Function*>(node->parent()->parent());
    assert(f);
    if (f->type().defined()) {
        bish_assert(f->type() == node->value->type()) <<
            "Invalid return type for function " << node->debug_info();
    } else {
        f->set_type(node->value->type());
    }
}

void TypeChecker::visit(ForLoop *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);

    node->variable->accept(this);
    node->lower->accept(this);
    if (node->upper) node->upper->accept(this);

    if (node->upper) {
        bish_assert(node->lower->type() == node->upper->type()) <<
            "Type mismatch for lower and upper loop bounds " << node->debug_info();
    }

    Type ty = node->lower->type().array() ? node->lower->type().element() : node->lower->type();
    node->variable->set_type(ty);

    node->body->accept(this);
}

void TypeChecker::visit(FunctionCall *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    unsigned i = 0;
    bish_assert(node->function->name != module->main->name) <<
        "Cannot call default 'main' function directly " << node->debug_info();
    bish_assert(node->function->body != NULL) <<
        "Calling an undefined function " << node->debug_info();
    for (std::vector<Assignment *>::const_iterator I = node->args.begin(),
             E = node->args.end(); I != E; ++I, ++i) {
        (*I)->accept(this);
        if (node->function->args[i]->type().defined()) {
            bish_assert((*I)->type() == node->function->args[i]->type()) <<
                "Invalid argument type for function call " << node->debug_info();
        } else {
            node->function->args[i]->set_type((*I)->type());
        }
    }
    node->function->accept(this);
    node->set_type(node->function->type());
}

void TypeChecker::visit(ExternCall *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    node->set_type(Type::Undef());
}

void TypeChecker::visit(IORedirection *node) {
    if (node->type().defined()) return;
    node->a->accept(this);
    node->b->accept(this);
    node->set_type(Type::Undef());
}

void TypeChecker::visit(Assignment *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    node->location->accept(this);
    Type ty = Type::Undef();
    for (std::vector<IRNode *>::const_iterator I = node->values.begin(),
             E = node->values.end(); I != E; ++I) {
        (*I)->accept(this);
        if (ty.defined()) {
            bish_assert((*I)->type() == ty) <<
                "Mixed types in array assignment " << node->debug_info();
        } else {
            ty = (*I)->type();
        }
    }
    Location *loc = node->location;
    bool array_initialization = node->values.size() > 1;
    Type dest_ty = loc->is_array_ref() && loc->variable->type().defined() ? loc->variable->type().element() : loc->variable->type();
    Type array_ty = Type::Array(ty);
    if (dest_ty.defined() && ty.defined()) {
        if (array_initialization) {
            bish_assert(dest_ty == array_ty) <<
                "Invalid type in array assignment " << node->debug_info();
        } else {
            bish_assert(dest_ty == ty) <<
                "Invalid type in assignment " << node->debug_info();
        }
    } else {
        if (loc->variable->type().undef()) {
            loc->variable->set_type(array_initialization ? array_ty : ty);
        } else {
            dest_ty = loc->is_array_ref() ? loc->variable->type().element() : loc->variable->type();
            if (array_initialization) {
                bish_assert(dest_ty == array_ty) <<
                    "Invalid type in reassignment " << node->debug_info();
            } else {
                bish_assert(dest_ty == ty) <<
                    "Invalid type in reassignment " << node->debug_info() <<
                    "\nexpected " << dest_ty.str() << " got " << ty.str();
            }
        }
    }
    if (loc->type().undef()) {
        loc->set_type(loc->is_array_ref() ? loc->variable->type().element() : loc->variable->type());
    }
    node->set_type(loc->type());
}

void TypeChecker::visit(BinOp *node) {
    if (visited(node) || node->type().defined()) return;
    visited_set.insert(node);
    node->a->accept(this);
    node->b->accept(this);
    propagate_if_undef(node->a, node->b);
    bish_assert(node->a->type() == node->b->type()) <<
        "Invalid operand types for binary operator " << node->debug_info();
    switch (node->op) {
    case BinOp::Eq:
    case BinOp::NotEq:
    case BinOp::LT:
    case BinOp::LTE:
    case BinOp::GT:
    case BinOp::GTE:
    case BinOp::And:
    case BinOp::Or:
        node->set_type(Type::Boolean());
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
    node->set_type(Type::Integer());
}

void TypeChecker::visit(Fractional *node) {
    node->set_type(Type::Fractional());
}

void TypeChecker::visit(String *node) {
    node->set_type(Type::String());
}

void TypeChecker::visit(Boolean *node) {
    node->set_type(Type::Boolean());
}

void TypeChecker::propagate_if_undef(IRNode *a, IRNode *b) {
    if (a->type().undef()) {
        a->set_type(b->type());
    } else if (b->type().undef()) {
        b->set_type(a->type());
    }
}
