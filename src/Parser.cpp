#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Errors.h"
#include "Util.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "LinkImportsPass.h"
#include "IRAncestorsPass.h"

namespace Bish {

void ParseScope::set_module(Module *m) {
    current_module = m;
}

void ParseScope::pop_module() {
    current_module = NULL;
}

Module *ParseScope::module() {
    return current_module;
}

void ParseScope::push_symbol_table() {
    symbol_table_stack.push(new SymbolTable());
}

void ParseScope::pop_symbol_table() {
    delete symbol_table_stack.top();
    symbol_table_stack.pop();
}

void ParseScope::add_symbol(const Name &name, Variable *v) {
    symbol_table_stack.top()->insert(name, v);
}

// Return the variable from the symbol table corresponding to the
// given variable. The given variable is then deleted. If there is no
// symbol table entry, abort.
Variable *ParseScope::get_defined_variable(Variable *v) {
    Variable *sym = lookup_variable(v->name);
    if (!sym) {
        bish_abort() << "Undefined variable \"" << v->name.name << "\"";
    }
    bish_assert(sym != v);
    delete v;
    return sym;
}

// Return the symbol table entry corresponding to the given variable
// name, or NULL if none exists.
Variable *ParseScope::lookup_variable(const Name &name) {
    IRNode *result = NULL;
    std::stack<SymbolTable *> aux;
    while (!symbol_table_stack.empty()) {
        SymbolTableEntry *e = symbol_table_stack.top()->lookup(name);
        if (e) {
            result = e->node;
            break;
        }
        aux.push(symbol_table_stack.top());
        symbol_table_stack.pop();
    }
    while (!aux.empty()) {
        symbol_table_stack.push(aux.top());
        aux.pop();
    }
    Variable *v = dynamic_cast<Variable*>(result);
    if (result) bish_assert(v);
    return v;
}

// Return the symbol table entry corresponding to the given function
// name, or NULL if none exists.
Function *ParseScope::lookup_function(const Name &name) {
    SymbolTableEntry *e = function_symbol_table->lookup(name);
    if (e) {
        Function *f = dynamic_cast<Function*>(e->node);
        assert(f);
        return f;
    } else {
        return NULL;
    }
}

// Return the symbol table entry corresponding to the given variable
// name. If no entry exists, create one first.
Variable *ParseScope::lookup_or_new_var(const Name &name) {
    Variable *result = lookup_variable(name);
    if (result == NULL) {
        result = new Variable(name);
        add_symbol(name, result);
    }
    bish_assert(result);
    return result;
}

// Return the symbol table entry corresponding to the given function
// name. If no entry exists, create one first.
Function *ParseScope::lookup_or_new_function(const Name &name) {
    Function *f = lookup_function(name);
    if (f == NULL) {
        f = new Function(name);
        function_symbol_table->insert(name, f);
    }
    bish_assert(f);
    return f;
}

Parser::~Parser() {
    if (tokenizer) delete tokenizer;
}

// Read all contents from the given input stream and return it as a
// string.
std::string Parser::read_stream(std::istream &is) {
    std::stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
}

// Return the entire contents of the file at the given path.
std::string Parser::read_file(const std::string &path) {
    std::ifstream t(path.c_str());
    if (!is_file(path) || !t.is_open()) {
        bish_abort() << "Failed to open file at " << path;
    }
    std::string result = read_stream(t);
    t.close();
    return result;
}

// Parse the given file into Bish IR.
Module *Parser::parse(const std::string &path) {
    std::string contents = read_file(path);
    Module *m = parse_string(contents, path);
    bish_assert(m->path.size() > 0) << "Unable to resolve module path";
    return m;
}

// Parse the given input stream into Bish IR.
Module *Parser::parse(std::istream &is) {
    std::string contents = read_stream(is);
    Module *m = parse_string(contents);
    return m;
}

// Parse the given string into Bish IR. If a path is given, set the
// resulting Module's path to that value.
Module *Parser::parse_string(const std::string &text, const std::string &path) {
    if (tokenizer) delete tokenizer;

    // Insert a dummy block for root scope.
    std::string preprocessed = "{\n" + text + "\n}";
    tokenizer = new Tokenizer(path, preprocessed);

    Module *m = module(path);
    expect(tokenizer->peek(), Token::EOSType, "Expected end of string.");

    post_parse_passes(m);
    return m;
}

// Run an ordered list of postprocessing passes over the IR.
void Parser::post_parse_passes(Module *m) {
    // Link modules from import statements.
    LinkImportsPass link;
    m->accept(&link);

    // Construct IRNode hierarchy
    IRAncestorsPass ancestors;
    m->accept(&ancestors);
}

// Assert that the given token is of the given type. If true, advance
// the tokenizer. If false, produce an error message.
void Parser::expect(const Token &t, Token::Type ty, const std::string &msg) {
    if (!t.isa(ty)) {
        abort_with_position(msg);
    }
    tokenizer->next();
}

// Wrapper around tokenizer->scan_until() that throws an error message
// if EOS is encountered.
std::string Parser::scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash) {
    std::string result = tokenizer->scan_until(tokens, keep_literal_backslash);
    if (tokenizer->peek().isa(Token::EOSType)) {
        bish_abort() << "Unexpected end of input.";
    }
    return result;
}

// Wrapper around tokenizer->scan_until() that throws an error message
// if EOS is encountered.
std::string Parser::scan_until(Token a, Token b) {
    std::vector<Token> tokens;
    tokens.push_back(a);
    tokens.push_back(b);
    return scan_until(tokens);
}

// Wrapper around tokenizer->scan_until() that throws an error message
// if EOS is encountered.
std::string Parser::scan_until(Token a) {
    std::vector<Token> tokens;
    tokens.push_back(a);
    return scan_until(tokens);
}

// Wrapper around tokenizer->scan_until() that throws an error message
// if EOS is encountered.
std::string Parser::scan_until(char c) {
    std::string result = tokenizer->scan_until(c);
    if (tokenizer->peek().isa(Token::EOSType)) {
        bish_abort() << "Unexpected end of input.";
    }
    return result;
}

// Terminate the parsing process with the given error message, and the
// position of the tokenizer.
void Parser::abort_with_position(const std::string &msg) {
    bish_abort() << ": parsing error: " << msg << " near " << tokenizer->position() << "\n";
}

Module *Parser::module(const std::string &path) {
    Module *m = new Module();
    m->set_path(path);
    scope.set_module(m);
    Function *main = new Function(Name("main"), block());
    m->set_main(main);
    setup_global_variables(m);
    scope.pop_module();
    return m;
}

// Turn module-level assignments into global variables.
void Parser::setup_global_variables(Module *m) {
    // The first assignment to a variable at module scope becomes the
    // global variable initializer. All later assignments are kept, as
    // they will go into the main function.
    std::vector<Block::iterator> to_erase;
    std::set<Variable *> handled;
    bish_assert(m->main != NULL);
    for (Block::iterator I = m->main->body->begin(), E = m->main->body->end(); I != E; ++I) {
        if (Assignment *a = dynamic_cast<Assignment*>(*I)) {
            Location *loc = a->location;
            if (handled.find(loc->variable) == handled.end()) {
                handled.insert(loc->variable);
                loc->variable->global = true;
                m->add_global(a);
                to_erase.push_back(I);
            }
        }
    }
    for (std::vector<Block::iterator>::iterator I = to_erase.begin(), E = to_erase.end(); I != E; ++I) {
        Block::iterator II = *I;
        m->main->body->nodes.erase(II);
    }
}

// Parse a Bish block.
Block *Parser::block() {
    std::vector<IRNode *> statements;
    scope.push_symbol_table();
    expect(tokenizer->peek(), Token::LBraceType, "Expected block to begin with '{'");
    do {
        while (tokenizer->peek().isa(Token::SharpType)) {
            scan_until('\n');
        }
        if (tokenizer->peek().isa(Token::RBraceType)) break;
        IRNode *s = stmt();
        if (s) statements.push_back(s);
    } while (!tokenizer->peek().isa(Token::RBraceType));
    expect(tokenizer->peek(), Token::RBraceType, "Expected block to end with '}'");
    Block *result = new Block(statements);
    scope.pop_symbol_table();
    return result;
}

IRNode *Parser::stmt() {
    Token t = tokenizer->peek();
    switch (t.type()) {
    case Token::LBraceType:
        return block();
    case Token::AtType: {
        IRNode *a = externcall();
        expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
        return a;
    }
    case Token::ImportType:
        return importstmt();
    case Token::ReturnType:
        return returnstmt();
    case Token::BreakType:
        return breakstmt();
    case Token::ContinueType:
        return continuestmt();
    case Token::IfType:
        return ifstmt();
    case Token::ForType:
        return forloop();
    case Token::DefType: {
        Function *f = functiondef();
        scope.module()->add_function(f);
        return NULL;
    }
    default:
        return otherstmt();
    }
}

IRNode *Parser::otherstmt() {
    Name sym = namespacedvar();
    IRNode *s = NULL;
    switch (tokenizer->peek().type()) {
    case Token::LBracketType:
    case Token::EqualsType:
        s = assignment(sym);
        break;
    case Token::LParenType:
        s = funcall(sym);
        break;
    default:
        abort_with_position("Unexpected token in statement.");
        s = NULL;
        break;
    }
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return s;
}

Assignment *Parser::assignment(const Name &name) {
    tokenizer->start_debug_info();
    bish_assert(!scope.lookup_function(name)) << "Cannot assign to function \"" <<
        name.str() << "\" near " << tokenizer->position();
    Variable *v = scope.lookup_or_new_var(name);
    scope.add_symbol(name, v);
    IRNode *offset = NULL;
    if (tokenizer->peek().isa(Token::LBracketType)) {
        tokenizer->next();
        offset = expr();
        expect(tokenizer->peek(), Token::RBracketType, "Expected matching ']'");
    }

    expect(tokenizer->peek(), Token::EqualsType, "Expected assignment operator");

    std::vector<IRNode *> values;
    if (tokenizer->peek().isa(Token::LBracketType)) {
        tokenizer->next();
        values = exprlist();
        expect(tokenizer->peek(), Token::RBracketType, "Expected matching ']'");
    } else {
        values.push_back(expr());
    }

    Location *loc = new Location(v, offset);
    return new Assignment(loc, values, tokenizer->end_debug_info());
}

FunctionCall *Parser::funcall(const Name &name) {
    tokenizer->start_debug_info();
    bish_assert(!scope.lookup_variable(name)) << "Symbol \"" <<
        name.str() << "\" near " << tokenizer->position() << " is not a function.";
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    std::vector<IRNode *> args;
    if (!tokenizer->peek().isa(Token::RParenType)) {
        args = exprlist();
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    Function *f = scope.lookup_or_new_function(name);
    return new FunctionCall(f, args, tokenizer->end_debug_info());
}

ExternCall *Parser::externcall() {
    tokenizer->start_debug_info();
    expect(tokenizer->peek(), Token::AtType, "Expected '@' to begin extern call.");
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    InterpolatedString *body = interpolated_string(Token::RParen(), false);
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    return new ExternCall(body, tokenizer->end_debug_info());
}

ImportStatement *Parser::importstmt() {
    tokenizer->start_debug_info();
    expect(tokenizer->peek(), Token::ImportType, "Expected import statement");
    std::string module_name = strip(scan_until(Token::Semicolon()));
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    if (namespaces.find(module_name) == namespaces.end()) {
        namespaces.insert(module_name);
        return new ImportStatement(scope.module(), module_name, tokenizer->end_debug_info());
    } else {
        // Ignore duplicate imports.
        tokenizer->end_debug_info();
        return NULL;
    }
}

ReturnStatement *Parser::returnstmt() {
    tokenizer->start_debug_info();
    expect(tokenizer->peek(), Token::ReturnType, "Expected return statement");
    IRNode *ret = expr();
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return new ReturnStatement(ret, tokenizer->end_debug_info());
}

LoopControlStatement *Parser::breakstmt() {
    tokenizer->start_debug_info();
    expect(tokenizer->peek(), Token::BreakType, "Expected break statement");
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return new LoopControlStatement(LoopControlStatement::Break, tokenizer->end_debug_info());
}

LoopControlStatement *Parser::continuestmt() {
    tokenizer->start_debug_info();
    expect(tokenizer->peek(), Token::ContinueType, "Expected continue statement");
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return new LoopControlStatement(LoopControlStatement::Continue, tokenizer->end_debug_info());
}

IfStatement *Parser::ifstmt() {
    expect(tokenizer->peek(), Token::IfType, "Expected if statement");
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    IRNode *cond = expr();
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    IRNode *body = block();
    std::vector<PredicatedBlock *> elses;
    IRNode *elseblock = NULL;
    while (tokenizer->peek().isa(Token::ElseType)) {
        tokenizer->next();
        if (tokenizer->peek().isa(Token::IfType)) {
            tokenizer->next();
            expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
            IRNode *econd = expr();
            expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
            IRNode *ebody = block();
            elses.push_back(new PredicatedBlock(econd, ebody));
        } else {
            elseblock = block();
        }
    }
    return new IfStatement(cond, body, elses, elseblock);
}

ForLoop *Parser::forloop() {
    expect(tokenizer->peek(), Token::ForType, "Expected for statement");
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    Variable *v = var();
    expect(tokenizer->peek(), Token::InType, "Expected keyword 'in'");
    IRNode *lower = atom(), *upper = NULL;
    if (tokenizer->peek().isa(Token::DoubleDotType)) {
        tokenizer->next();
        upper = atom();
    }
    if (Variable *lv = dynamic_cast<Variable*>(lower)) {
        lower = scope.get_defined_variable(lv);
    }
    if (Variable *uv = dynamic_cast<Variable*>(upper)) {
        upper = scope.get_defined_variable(uv);
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    IRNode *body = block();
    return new ForLoop(v, lower, upper, body);
}

Function *Parser::functiondef() {
    expect(tokenizer->peek(), Token::DefType, "Expected def statement");
    Name name = namespacedvar();
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    std::vector<Variable *> args;
    scope.push_symbol_table();
    if (!tokenizer->peek().isa(Token::RParenType)) {
        args.push_back(arg());
        while (tokenizer->peek().isa(Token::CommaType)) {
            tokenizer->next();
            args.push_back(arg());
        }
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    Block *body = block();
    scope.pop_symbol_table();
    // It's possible the function was called before it was defined. In
    // that case we get that function instance, and initialize its
    // arguments and body here.
    Function *f = scope.lookup_or_new_function(name);
    // If the arguments and body have already been initialized, throw
    // a redefinition error.
    if (f->body != NULL) {
        abort_with_position("Function '" + name.name + "' is already defined.");
    }
    f->set_args(args);
    f->set_body(body);
    return f;
}

IRNode *Parser::expr() {
    tokenizer->start_debug_info();
    IRNode *a = logical();
    Token t = tokenizer->peek();
    if (t.isa(Token::PipeType)) {
        tokenizer->next();
        a = new IORedirection(get_redirection_operator(t), a, logical(), tokenizer->end_debug_info());
    } else {
        tokenizer->end_debug_info();
    }
    return a;
}

std::vector<IRNode *> Parser::exprlist() {
    std::vector<IRNode *> result;
    result.push_back(expr());
    while (tokenizer->peek().isa(Token::CommaType)) {
        tokenizer->next();
        result.push_back(expr());
    }
    return result;
}

IRNode *Parser::logical() {
    tokenizer->start_debug_info();
    IRNode *a = equality();
    Token t = tokenizer->peek();
    while (t.isa(Token::AndType) || t.isa(Token::OrType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, equality(), tokenizer->end_debug_info());
        tokenizer->start_debug_info();
        t = tokenizer->peek();
    }
    tokenizer->end_debug_info();
    return a;
}

IRNode *Parser::equality() {
    tokenizer->start_debug_info();
    IRNode *a = relative();
    Token t = tokenizer->peek();
    if (t.isa(Token::DoubleEqualsType) || t.isa(Token::NotEqualsType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, relative(), tokenizer->end_debug_info());
        t = tokenizer->peek();
    } else {
        tokenizer->end_debug_info();
    }
    return a;
}

IRNode *Parser::relative() {
    tokenizer->start_debug_info();
    IRNode *a = arith();
    Token t = tokenizer->peek();
    if (t.isa(Token::LAngleType) || t.isa(Token::LAngleEqualsType) ||
        t.isa(Token::RAngleType) || t.isa(Token::RAngleEqualsType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, arith(), tokenizer->end_debug_info());
        t = tokenizer->peek();
    } else {
        tokenizer->end_debug_info();
    }
    return a;
}

IRNode *Parser::arith() {
    tokenizer->start_debug_info();
    IRNode *a = term();
    Token t = tokenizer->peek();
    while (t.isa(Token::PlusType) || t.isa(Token::MinusType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, term(), tokenizer->end_debug_info());
        tokenizer->start_debug_info();
        t = tokenizer->peek();
    }
    tokenizer->end_debug_info();
    return a;
}

IRNode *Parser::term() {
    tokenizer->start_debug_info();
    IRNode *a = unary();
    Token t = tokenizer->peek();
    while (t.isa(Token::StarType) || t.isa(Token::SlashType) || t.isa(Token::PercentType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, unary(), tokenizer->end_debug_info());
        tokenizer->start_debug_info();
        t = tokenizer->peek();
    }
    tokenizer->end_debug_info();
    return a;
}

IRNode *Parser::unary() {
    tokenizer->start_debug_info();
    Token t = tokenizer->peek();
    if (is_unop_token(t)) {
        tokenizer->next();
        return new UnaryOp(get_unaryop_operator(t), factor(), tokenizer->end_debug_info());
    } else {
        tokenizer->end_debug_info();
        return factor();
    }
}

IRNode *Parser::factor() {
    if (tokenizer->peek().isa(Token::LParenType)) {
        tokenizer->next();
        IRNode *e = expr();
        expect(tokenizer->peek(), Token::RParenType, "Unmatched '('");
        return e;
    } else if (tokenizer->peek().isa(Token::AtType)) {
        return externcall();
    } else {
        IRNode *a = atom();
        if (tokenizer->peek().isa(Token::LParenType)) {
            Location *loc = dynamic_cast<Location*>(a);
            if (loc == NULL) {
                abort_with_position("Invalid atom type for function call");
            }
            a = funcall(loc->variable->name);
        } else if (Location *loc = dynamic_cast<Location*>(a)) {
            Variable *sym = scope.get_defined_variable(loc->variable);
            loc->variable = sym;
        }
        return a;
    }
}

IRNode *Parser::atom() {
    Token t = tokenizer->peek();
    tokenizer->next();

    switch(t.type()) {
    case Token::SymbolType: {
        Variable *v = NULL;
        IRNode *offset = NULL;
        if (tokenizer->peek().isa(Token::DotType)) {
            tokenizer->next();
            Token t1 = tokenizer->peek();
            expect(t1, Token::SymbolType, "Expected symbol.");
            if (namespaces.find(t.value()) == namespaces.end()) {
                abort_with_position("Unknown namespace");
            }
            v = new Variable(Name(t1.value(), t.value()));
        } else {
            v = new Variable(Name(t.value()));
        }
        if (tokenizer->peek().isa(Token::LBracketType)) {
            tokenizer->next();
            offset = expr();
            expect(tokenizer->peek(), Token::RBracketType, "Expected matching ']'");
        }
        assert(v);
        return new Location(v, offset);
    }
    case Token::TrueType:
        return new Boolean(true);
    case Token::FalseType:
        return new Boolean(false);
    case Token::IntType:
        return new Integer(t.value());
    case Token::FractionalType:
        return new Fractional(t.value());
    case Token::QuoteType: {
        InterpolatedString *str = interpolated_string(Token::Quote(), true);
        expect(tokenizer->peek(), Token::QuoteType, "Unmatched '\"'");
        return new String(str);
    }
    default:
        abort_with_position("Invalid token type for atom.");
        return NULL;
    }
}

Variable *Parser::var() {
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected variable to be a symbol");
    return scope.lookup_or_new_var(name);
}

Variable *Parser::arg() {
    // Similar to var() but this always creates a new symbol.
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected argument to be a symbol");
    Variable *arg = new Variable(name);
    scope.add_symbol(name, arg);
    return arg;
}

Name Parser::namespacedvar() {
    Token t = tokenizer->peek();
    expect(t, Token::SymbolType, "Expected symbol.");
    if (tokenizer->peek().isa(Token::DotType)) {
        tokenizer->next();
        Token t1 = tokenizer->peek();
        expect(t1, Token::SymbolType, "Expected symbol.");
        if (namespaces.find(t.value()) == namespaces.end()) {
            abort_with_position("Unknown namespace");
        }
        return Name(t1.value(), t.value());
    }
    return Name(t.value());
}

// Parse an interpolated string. The given token parameter is the
// stopping token. E.g. if the interpolated string should be parsed
// between double quotes, the caller would consume the initial double
// quote and call this function with stop = Token::Quote().
InterpolatedString *Parser::interpolated_string(const Token &stop, bool keep_literal_backslash) {
    const Token scan_tokens_arr[] = {stop, Token::Dollar()};
    const std::vector<Token> scan_tokens(scan_tokens_arr, scan_tokens_arr+2);
    InterpolatedString *result = new InterpolatedString();
    while (true) {
        std::string str = scan_until(scan_tokens, keep_literal_backslash);
        result->push_str(str);
        if (tokenizer->peek().isa(Token::DollarType)) {
            tokenizer->next();
            if (tokenizer->peek().isa(Token::LParenType)) {
                tokenizer->next();
                str = scan_until(Token::RParen());
                tokenizer->next();
                result->push_str("$" + str);
            } else {
                Variable *v = var();
                result->push_var(v);
            }
        } else if (tokenizer->peek().isa(stop.type())) {
            break;
        }
    }
    return result;
}

}
