#include <cassert>
#include "IR.h"
#include "ReplaceIRNodes.h"

using namespace Bish;

namespace {
template <class T>
T assert_cast(IRNode *n) {
    T t = dynamic_cast<T>(n);
    assert(t != NULL);
    return t;
}
}

IRNode *ReplaceIRNodes::replacement(IRNode *node) {
    std::map<IRNode *, IRNode *>::iterator I = replace_map.find(node);
    if (I == replace_map.end()) {
	return NULL;
    } else {
	return I->second;
    }
}

void ReplaceIRNodes::visit(Module *node) {
    for (unsigned i = 0; i < node->global_variables.size(); i++) {
	if (IRNode *n = replacement(node->global_variables[i])) {
	    Assignment *a = assert_cast<Assignment*>(n);
	    node->global_variables[i] = a;
	}
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(Block *node) {
    for (unsigned i = 0; i < node->nodes.size(); i++) {
	if (IRNode *n = replacement(node->nodes[i])) {
	    node->nodes[i] = n;
	}	
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(FunctionCall *node) {
    for (unsigned i = 0; i < node->args.size(); i++) {
	if (IRNode *n = replacement(node->args[i])) {
	    Assignment *a = assert_cast<Assignment*>(n);
	    node->args[i] = a;
	}
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(IORedirection *node) {
    if (IRNode *na = replacement(node->a)) {
	node->a = na;
    }
    if (IRNode *nb = replacement(node->b)) {
	node->b = nb;
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(IfStatement *node) {
    for (std::vector<PredicatedBlock *>::const_iterator I = node->elses.begin(),
             E = node->elses.end(); I != E; ++I) {
	if (IRNode *n = replacement((*I)->condition)) {
	    (*I)->condition = n;
	}
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(ReturnStatement *node) {
    if (IRNode *n = replacement(node->value)) {
	node->value = n;
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(ForLoop *node) {
    if (IRNode *n = replacement(node->variable)) {
	Variable *v = assert_cast<Variable*>(n);
	node->variable = v;
    }
    if (IRNode *n = replacement(node->lower)) {
	node->lower = n;
    }
    if (IRNode *n = replacement(node->upper)) {
	node->upper = n;
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(Assignment *node) {
    if (IRNode *n = replacement(node->location)) {
	Location *l = assert_cast<Location*>(n);
	node->location = l;
    }
    for (unsigned i = 0; i < node->values.size(); i++) {
	if (IRNode *n = replacement(node->values[i])) {
	    node->values[i] = n;
	}
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(BinOp *node) {
    if (IRNode *na = replacement(node->a)) {
	node->a = na;
    }
    if (IRNode *nb = replacement(node->b)) {
	node->b = nb;
    }
    IRVisitor::visit(node);
}

void ReplaceIRNodes::visit(UnaryOp *node) {
    if (IRNode *na = replacement(node->a)) {
	node->a = na;
    }
    IRVisitor::visit(node);
}

