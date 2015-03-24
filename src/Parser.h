#ifndef __BISH_PARSER_H__
#define __BISH_PARSER_H__

#include <stack>
#include <string>
#include "IR.h"
#include "SymbolTable.h"
#include "Tokenizer.h"

/*
Grammar:

module ::= block
block ::= '{' { stmt } '}'
stmt ::= assign ';'
       | funcall ';'
       | externcall ';'
       | 'import' any ';'
       | 'return' expr ';'
       | 'break' ';'
       | 'continue' ';'
       | '#' any NEWLINE
       | 'if' '(' expr ')' block
       | 'if' '(' expr ')' block { 'else' 'if' '(' expr ')' block } 'else' block
       | 'for' '(' var 'in' atom [ '..' atom ] ')' block
       | 'def' var '(' varlist ')' block
       | block
assign ::= namespacedvar '=' expr
expr ::= expr '|' logical | logical
logical ::= logical 'and' equality | logical 'or' equality | equality
equality ::= equality '==' relative | equality '!=' relative | relative
relative ::= relative '<' arith | relative '>' arith
           | relative '<=' arith | relative '>=' arith
           | arith
arith ::= arith '+' term | arith '-' term | term
term ::= term '*' unary | term '/' unary | term '%' unary | unary
unary ::= '-' unary | 'not' unary | factor
factor ::= '( expr ')' | funcall | externcall | atom
funcall ::= namespacedvar '(' exprlist ')'
externcall ::= '@' '(' interp ')'
atom ::= namespacedvar | NUMBER | '"' STRING '"' | 'true' | 'false'
var ::= { ALPHANUM | '_' }
namespacedvar ::= [ var '.' ] var
varlist ::= var { ',' var }
atomlist ::= expr { ',' expr }
interp ::= { str | '$' namespacedvar | '$' '(' any ')'}
*/

namespace Bish {

class Parser {
public:
    Parser() : tokenizer(NULL) {}
    ~Parser();
    Module *parse(const std::string &path);
    Module *parse(std::istream &is);
    Module *parse_string(const std::string &text, const std::string &path="");
private:
    Tokenizer *tokenizer;
    std::set<std::string> namespaces;
    std::stack<Module *> module_stack;
    std::stack<SymbolTable *> symbol_table_stack;
    SymbolTable *function_symbol_table;

    std::string read_stream(std::istream &is);
    std::string read_file(const std::string &path);
    void abort(const std::string &msg);
    void abort_with_position(const std::string &msg);
    void expect(const Token &t, Token::Type ty, const std::string &msg);
    std::string scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash=true);
    std::string scan_until(Token a, Token b);
    std::string scan_until(Token t);
    std::string scan_until(char c);
    void push_module(Module *m);
    Module *pop_module();
    void push_symbol_table(SymbolTable *s);
    SymbolTable *pop_symbol_table();
    void setup_global_variables(Module *m);
    Variable *get_defined_variable(Variable *v);
    Variable *lookup_variable(const Name &name);
    Variable *lookup_or_new_var(const Name &name);
    Function *lookup_or_new_function(const Name &name);
    void post_parse_passes(Module *m);

    Module *module(const std::string &path);
    Block *block();
    IRNode *stmt();
    IRNode *otherstmt();
    IRNode *ifstmt();
    IRNode *importstmt();
    IRNode *returnstmt();
    IRNode *breakstmt();
    IRNode *continuestmt();
    IRNode *forloop();
    Function *functiondef();
    IRNode *externcall();
    InterpolatedString *interpolated_string(const Token &stop, bool keep_literal_backslash);
    IRNode *funcall(const Name &name);
    IRNode *assignment(const Name &name);
    Variable *var();
    Variable *arg();
    IRNode *expr();
    IRNode *logical();
    IRNode *equality();
    IRNode *relative();
    IRNode *arith();
    IRNode *term();
    IRNode *unary();
    IRNode *factor();
    IRNode *atom();
    Name symbol();

};

}
#endif
