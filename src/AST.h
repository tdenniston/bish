#ifndef __BISH_AST_H__
#define __BISH_AST_H__

namespace Bish {

class ASTNode {

};

class AST {
public:
    AST() : root(NULL) {}
    
private:
    ASTNode *root;
};

}
#endif
