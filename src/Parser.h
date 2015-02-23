#ifndef __BISH_PARSER_H__
#define __BISH_PARSER_H__

#include <string>
#include "AST.h"

/*
Grammar:

block ::= '{' { stmt ';' } '}'
stmt ::= var '=' expr
var ::= NAME
expr ::= '(' expr ')' | unop atom | atom { binop atom }
binop ::= '+' | '-' | '*' | '/'
unop ::= '-'
atom ::= var | NUMBER | true | false
*/

namespace Bish {

class Tokenizer;

class Token {
public:
    typedef enum { LParenType,
                   RParenType,
                   LBraceType,
                   RBraceType,
                   SemicolonType,
                   EqualsType,
                   PlusType,
                   MinusType,
                   StarType,
                   SlashType,
                   SymbolType,
                   IntType,
                   FractionalType,
                   EOSType,
                   NoneType } Type;

    Token() : type(NoneType) {}
    Token(Type t) : type(t) {}
    Token(Type t, const std::string &s) : type(t), str_value(s) {}

    bool defined() const { return type != NoneType; }
    bool isa(Type t) const { return type == t; }
    const std::string &value() const { return str_value; }

    static Token LParen() {
        return Token(LParenType, "(");
    }

    static Token RParen() {
        return Token(RParenType, ")");
    }

    static Token LBrace() {
        return Token(LBraceType, "{");
    }

    static Token RBrace() {
        return Token(RBraceType, "}");
    }

    static Token Semicolon() {
        return Token(SemicolonType, ";");
    }

    static Token Equals() {
        return Token(EqualsType, "=");
    }

    static Token Plus() {
        return Token(PlusType, "+");
    }
    
    static Token Minus() {
        return Token(MinusType, "-");
    }
    
    static Token Star() {
        return Token(StarType, "*");
    }
    
    static Token Slash() {
        return Token(SlashType, "/");
    }
    
    static Token Symbol(const std::string &s) {
        return Token(SymbolType, s);
    }

    static Token Int(const std::string &s) {
        return Token(IntType, s);
    }

    static Token Fractional(const std::string &s) {
        return Token(FractionalType, s);
    }

    static Token EOS() {
        return Token(EOSType, "<EOS>");
    }
private:
    Type type;
    std::string str_value;
};

class Parser {
public:
    Parser() : tokenizer(NULL), current_ast_node(NULL) {}
    ~Parser();
    AST *parse(const std::string &text);
private:
    Tokenizer *tokenizer;
    ASTNode *current_ast_node;
    void expect(const Token &t, Token::Type ty, const std::string &msg);
    bool is_unop_token(const Token &t);
    bool is_binop_token(const Token &t);
    void abort();
    void block();
    void stmt();
    void var();
    void expr();
    void binop();
    void unop();
    void atom();
};

}
#endif
