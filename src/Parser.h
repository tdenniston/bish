#ifndef __BISH_PARSER_H__
#define __BISH_PARSER_H__

#include <stack>
#include <string>
#include "IR.h"
#include "SymbolTable.h"

/*
Grammar:

module ::= block
block ::= '{' { stmt } '}'
stmt ::= assign ';'
       | funcall ';'
       | externcall ';'
       | 'return' expr ';'
       | '#' any NEWLINE
       | 'if' '(' expr ')' block
       | 'if' '(' expr ')' block { 'else' 'if' '(' expr ')' block } 'else' block
       | 'for' '(' var 'in' atom [ '..' atom ] ')' block
       | 'def' var '(' varlist ')' block
       | block
assign ::= var '=' expr
expr ::= expr '==' relative | expr '!=' relative | relative
relative ::= relative '<' arith | relative '>' arith
           | relative '<=' arith | relative '>=' arith
           | arith
arith ::= arith '+' term | arith '-' term | term
term ::= term '*' unary | term '/' unary | unary
unary ::= '-' unary | factor
factor ::= '( expr ')' | funcall | externcall | atom
funcall ::= var '(' exprlist ')'
externcall ::= '@' '(' interp ')'
atom ::= var | NUMBER | '"' STRING '"' | 'true' | 'false'
var ::= ALPHANUM
varlist ::= var { ',' var }
atomlist ::= expr { ',' expr }
interp ::= { str | '$' var | '$' '(' any ')'}
*/

namespace Bish {

class Tokenizer;

class Token {
public:
    typedef enum { LParenType,
                   RParenType,
                   LBraceType,
                   RBraceType,
                   LBracketType,
                   RBracketType,
                   AtType,
                   DollarType,
                   SharpType,
                   SemicolonType,
                   CommaType,
                   ReturnType,
                   IfType,
                   ElseType,
                   DefType,
                   ForType,
                   InType,
                   DoubleDotType,
                   EqualsType,
                   DoubleEqualsType,
                   NotEqualsType,
                   LAngleType,
                   LAngleEqualsType,
                   RAngleType,
                   RAngleEqualsType,
                   PlusType,
                   MinusType,
                   StarType,
                   SlashType,
                   QuoteType,
                   SymbolType,
                   TrueType,
                   FalseType,
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

    static Token LBracket() {
        return Token(LBracketType, "[");
    }

    static Token RBracket() {
        return Token(RBracketType, "]");
    }

    static Token At() {
        return Token(AtType, "@");
    }

    static Token Dollar() {
        return Token(DollarType, "$");
    }

    static Token Sharp() {
        return Token(SharpType, "#");
    }

    static Token Semicolon() {
        return Token(SemicolonType, ";");
    }

    static Token Comma() {
        return Token(CommaType, ",");
    }

    static Token Return() {
        return Token(ReturnType, "return");
    }

    static Token If() {
        return Token(IfType, "if");
    }

    static Token Else() {
        return Token(ElseType, "else");
    }

    static Token Def() {
        return Token(DefType, "def");
    }

    static Token For() {
        return Token(ForType, "for");
    }

    static Token In() {
        return Token(InType, "in");
    }

    static Token DoubleDot() {
        return Token(DoubleDotType, "..");
    }

    static Token Equals() {
        return Token(EqualsType, "=");
    }

    static Token DoubleEquals() {
        return Token(DoubleEqualsType, "==");
    }

    static Token NotEquals() {
        return Token(NotEqualsType, "!=");
    }

    static Token LAngle() {
        return Token(LAngleType, "<");
    }

    static Token LAngleEquals() {
        return Token(LAngleEqualsType, "<=");
    }

    static Token RAngle() {
        return Token(RAngleType, ">");
    }

    static Token RAngleEquals() {
        return Token(DoubleEqualsType, ">=");
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

    static Token True() {
        return Token(TrueType, "true");
    }

    static Token False() {
        return Token(FalseType, "false");
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
 Parser() : tokenizer(NULL) {}
    ~Parser();
    Module *parse(const std::string &path);
    Module *parse_string(const std::string &text);
private:
    Tokenizer *tokenizer;
    std::stack<Module *> module_stack;
    std::stack<SymbolTable *> symbol_table_stack;

    std::string read_file(const std::string &path);
    void abort(const std::string &msg);
    bool is_unop_token(const Token &t);
    bool is_binop_token(const Token &t);
    BinOp::Operator get_binop_operator(const Token &t);
    UnaryOp::Operator get_unaryop_operator(const Token &t);
    Type get_primitive_type(const IRNode *n);
    void expect(const Token &t, Token::Type ty, const std::string &msg);
    void push_module(Module *m);
    Module *pop_module();
    void push_symbol_table(SymbolTable *s);
    SymbolTable *pop_symbol_table();
    IRNode *lookup(const std::string &name);
    Variable *lookup_or_new_var(const std::string &name);
    void remove_from_symbol_table(const std::string &name);

    Module *module();
    Block *block();
    IRNode *stmt();
    IRNode *otherstmt();
    IRNode *ifstmt();
    IRNode *returnstmt();
    IRNode *forloop();
    Function *functiondef();
    IRNode *externcall();
    IRNode *funcall(const std::string &name);
    IRNode *assignment(const std::string &name);
    Variable *var();
    Variable *arg();
    IRNode *expr();
    IRNode *relative();
    IRNode *arith();
    IRNode *term();
    IRNode *unary();
    IRNode *factor();
    IRNode *atom();
    std::string symbol();

};

}
#endif
