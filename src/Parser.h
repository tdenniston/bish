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

/* Class which encapsulates all of the scoping information needed
 * during parsing (e.g. symbol tables). */
class ParseScope {
public:
    ParseScope() {
        function_symbol_table = new SymbolTable();
    }

    ~ParseScope() {
        delete function_symbol_table;
    }

    // Set the current module.
    void set_module(Module *m);
    // Unset the current module.
    void pop_module();
    // Return the current module.
    Module *module();
    // Create a new scope for variables.
    void push_symbol_table();
    // Remove the topmost scope for variables.
    void pop_symbol_table();
    // Add the given symbol to the current variable scope.
    void add_symbol(const Name &name, Variable *v, Type ty);
    // Return the variable from the symbol table corresponding to the
    // given variable. The given variable is then deleted. If there is no
    // symbol table entry, abort.
    Variable *get_defined_variable(Variable *v);
    // Return the symbol table entry corresponding to the given variable
    // name, or NULL if none exists.
    Variable *lookup_variable(const Name &name);
    // Return the symbol table entry corresponding to the given variable
    // name. If no entry exists, create one first.
    Variable *lookup_or_new_var(const Name &name);
    // Return the symbol table entry corresponding to the given function
    // name. If no entry exists, create one first.
    Function *lookup_or_new_function(const Name &name);
private:
    // Current Module being parsed.
    Module *current_module;
    // Current scope stack of symbol tables for variables.
    std::stack<SymbolTable *> symbol_table_stack;
    // Symbol table for functions (which are defined globally).
    SymbolTable *function_symbol_table;
};

class Parser {
public:
    Parser() : tokenizer(NULL) {}
    ~Parser();
    Module *parse(const std::string &path);
    Module *parse(std::istream &is);
    Module *parse_string(const std::string &text, const std::string &path="");
private:
    ParseScope scope;
    Tokenizer *tokenizer;
    std::set<std::string> namespaces;

    std::string read_stream(std::istream &is);
    std::string read_file(const std::string &path);
    void abort_with_position(const std::string &msg);
    void expect(const Token &t, Token::Type ty, const std::string &msg);
    std::string scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash=true);
    std::string scan_until(Token a, Token b);
    std::string scan_until(Token t);
    std::string scan_until(char c);
    void setup_global_variables(Module *m);
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
