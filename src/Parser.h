#ifndef __BISH_PARSER_H__
#define __BISH_PARSER_H__

#include <string>
#include "AST.h"
#include "SymbolTable.h"

/*
Grammar:

block ::= '{' { stmt } '}'
stmt ::= assign ';'
       | funcall ';'
       | 'if' '(' expr ')' block
       | 'def' var '(' varlist ')' block
       | block
assign ::= var '=' expr
expr ::= expr '==' arith | arith
arith ::= arith '+' term | arith '-' term | term
term ::= term '*' unary | term '/' unary | unary
unary ::= '-' unary | factor
factor ::= '( expr ')' | funcall | atom
funcall ::= var '(' atomlist ')'
atom ::= var | NUMBER | '"' STRING '"'
var ::= STRING
varlist ::= var { ',' var }
atomlist ::= atom { ',' atom }
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
                   CommaType,
                   IfType,
                   DefType,
                   EqualsType,
                   DoubleEqualsType,
                   PlusType,
                   MinusType,
                   StarType,
                   SlashType,
                   QuoteType,
                   SymbolType,
                   IntType,
                   FractionalType,
                   EOSType,
                   NoneType } Type;

    Token() : type_(NoneType) {}
    Token(Type t) : type_(t) {}
    Token(Type t, const std::string &s) : type_(t), value_(s) {}

    bool defined() const { return type_ != NoneType; }
    bool isa(Type t) const { return type_ == t; }
    Type type() const { return type_; }
    const std::string &value() const { return value_; }

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

    static Token Comma() {
        return Token(CommaType, ",");
    }

    static Token If() {
        return Token(IfType, "if");
    }

    static Token Def() {
        return Token(DefType, "def");
    }

    static Token Equals() {
        return Token(EqualsType, "=");
    }

    static Token DoubleEquals() {
        return Token(DoubleEqualsType, "==");
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

    static Token Quote() {
      return Token(QuoteType, "\"");
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
    Type type_;
    std::string value_;
};

class Parser {
public:
    Parser() : tokenizer(NULL), current_symbol_table(NULL) {}
    ~Parser();
    AST *parse(const std::string &path);
    AST *parse_string(const std::string &text);
private:
    Tokenizer *tokenizer;
    SymbolTable *current_symbol_table;
    
    std::string read_file(const std::string &path);
    void abort(const std::string &msg);
    bool is_unop_token(const Token &t);
    bool is_binop_token(const Token &t);
    BinOp::Operator get_binop_operator(const Token &t);
    UnaryOp::Operator get_unaryop_operator(const Token &t);
    Type get_primitive_type(const ASTNode *n);
    void expect(const Token &t, Token::Type ty, const std::string &msg);

    Block *block();
    ASTNode *stmt();
    ASTNode *otherstmt();
    ASTNode *ifstmt();
    ASTNode *functiondef();
    ASTNode *funcall(Variable *a);
    ASTNode *assignment(Variable *a);
    Variable *var();
    NodeList *nodelist();
    ASTNode *expr();
    ASTNode *arith();
    ASTNode *term();
    ASTNode *unary();
    ASTNode *factor();
    ASTNode *atom();
    
};

}
#endif
