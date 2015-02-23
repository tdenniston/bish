#include "AST.h"

namespace Bish {

ASTNode *ASTNode::root() const {
    ASTNode *n = parent();
    while (n->parent()) {
        n = n->parent();
    }
    return n;
}

ASTNode *ASTNode::parent() const {
    return parent_;
}

}
