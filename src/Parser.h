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
assign ::= location '=' expr
         | location '=' '[' exprlist ']'
funcall ::= namespacedvar '(' exprlist ')'
externcall ::= '@' '(' interp ')'
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
atom ::= location | NUMBER | '"' STRING '"' | 'true' | 'false'
var ::= { ALPHANUM | '_' }
location ::= namespacedvar | namespacedvar '[' expr ']'
namespacedvar ::= [ var '.' ] var
varlist ::= var { ',' var }
exprlist ::= expr { ',' expr }
interp ::= { str | '$' namespacedvar | '$' '(' any ')'}
*/

namespace Bish {

/* Class which encapsulates all of the scoping information needed
 * during parsing (e.g. symbol tables). */
class ParseScope {
public:
    ParseScope() {
        function_symbol_table = new SymbolTable();
        unique_id = 0;
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
    void add_symbol(const Name &name, Variable *v);
    // Return a name that is guaranteed to be unique.
    Name get_unique_name();
    // Return the variable from the symbol table corresponding to the
    // given variable. The given variable is then deleted. If there is no
    // symbol table entry, abort.
    Variable *get_defined_variable(Variable *v);
    // Return the symbol table entry corresponding to the given variable
    // name, or NULL if none exists.
    Variable *lookup_variable(const Name &name);
    // Return the symbol table entry corresponding to the given function
    // name, or NULL if none exists.
    Function *lookup_function(const Name &name);
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
    // Counter for unique names.
    unsigned unique_id;
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
    std::stack<Block *> block_stack;

    std::string read_stream(std::istream &is);
    std::string read_file(const std::string &path);
    void abort_with_position(const std::string &msg);
    void expect(const Token &t, Token::Type ty, const std::string &msg);
    std::string scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash=true);
    std::string scan_until(Token a, Token b);
    std::string scan_until(Token t);
    std::string scan_until(char c);
    void setup_builtin_symbols();
    void setup_global_variables(Module *m);
    void post_parse_passes(Module *m);
    void push_block(Block *b);
    void pop_block();
    
    Module *module(const std::string &path);
    Block *block();
    IRNode *stmt();
    IRNode *otherstmt();
    Assignment *assignment(const Name &name);
    FunctionCall *funcall(const Name &name);
    ExternCall *externcall();
    ImportStatement *importstmt();
    ReturnStatement *returnstmt();
    LoopControlStatement *breakstmt();
    LoopControlStatement *continuestmt();
    IfStatement *ifstmt();
    ForLoop *forloop();
    Function *functiondef();
    IRNode *expr();
    std::vector<IRNode *> exprlist();
    IRNode *logical();
    IRNode *equality();
    IRNode *relative();
    IRNode *arith();
    IRNode *term();
    IRNode *unary();
    IRNode *factor();
    IRNode *atom();
    Variable *var();
    Variable *arg();
    Name namespacedvar();
    InterpolatedString *interpolated_string(const Token &stop, bool keep_literal_backslash);

};

}
#endif
