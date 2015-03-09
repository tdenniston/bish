#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Util.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "IRAncestorsPass.h"

namespace {
inline bool is_newline(char c) {
    return c == '\n';
}

inline bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || is_newline(c);
}

inline bool is_digit(char c) {
    return c >= 0x30 && c <= 0x39;
}

inline bool is_alphanumeric(char c) {
    return is_digit(c) || (c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a);
}

}

namespace Bish {

/*
 * The Bish tokenizer. Given a string to tokenize, use the peek() and
 * next() methods to produce a stream of tokens.
 */
class Tokenizer {
public:
    Tokenizer(const std::string &t) : text(t), idx(0), lineno(1) {}

    // Return the token at the head of the stream, but do not skip it.
    Token peek() {
        ResultState st = get_token();
        return st.first;
    }

    // Skip the token currently at the head of the stream.
    void next() {
        ResultState st = get_token();
        idx = st.second;
    }

    // Return the substring beginning at the current index and
    // continuing  until the first occurrence of a token of type
    // a or b.
    std::string scan_until(Token a, Token b) {
        unsigned start = idx;
        char ca = a.value()[0], cb = b.value()[0];
        unsigned lena = a.value().size(), lenb = b.value().size();
        while (!eos()) {
            while (!eos() && curchar() != ca && curchar() != cb) {
                idx++;
            }
            if (lookahead(lena).compare(a.value()) == 0) {
                idx += lena - 1;
                break;
            } else if (lookahead(lenb).compare(b.value()) == 0) {
                idx += lenb - 1;
                break;
            }
        }
        return text.substr(start, idx - start);
    }

    // Return the substring beginning at the current index and
    // continuing  until the first occurrence of a token of type
    // t.
    std::string scan_until(Token t) {
        unsigned start = idx;
        char c = t.value()[0];
        unsigned len = t.value().size();
        while (!eos()) {
            while (!eos() && curchar() != c) {
                idx++;
            }
            if (lookahead(len).compare(t.value()) == 0) {
                idx += len - 1;
                break;
            }
        }
        return text.substr(start, idx - start);
    }

    // Return the substring beginning at the current index and
    // continuing until the first occurrence of a character of the
    // given value.
    std::string scan_until(char c) {
        unsigned start = idx;
        while (curchar() != c) {
            idx++;
        }
        return text.substr(start, idx - start);
    }

    // Return the substring beginning at the current index and
    // continuing (exclusively) until the first occurrence of a
    // non-whitespace character.
    std::string scan_whitespace() {
        unsigned start = idx;
        while (is_whitespace(curchar())) {
            idx++;
        }
        return text.substr(start, idx - start);
    }

    // Return a human-readable representation of the current position
    // in the string.
    std::string position() const {
        std::stringstream s;
        s << "character '" << text[idx] << "', line " << lineno;
        return s.str();
    }

private:
    typedef std::pair<Token, unsigned> ResultState;
    const std::string &text;
    unsigned idx;
    unsigned lineno;

    // Return the current character.
    inline char curchar() const {
        return text[idx];
    }

    inline char nextchar() const {
        return text[idx + 1];
    }

    inline std::string lookahead(int n) const {
        if (idx + n >= text.length()) return "";
        return text.substr(idx, n);
    }

    // Return true if the tokenizer is at "end of string".
    inline bool eos() const {
        return idx >= text.length();
    }

    // Skip ahead until the next non-whitespace character.
    inline void skip_whitespace() {
        while (!eos() && is_whitespace(curchar())) {
            if (is_newline(curchar())) ++lineno;
            ++idx;
        }
    }

    // Form the next token. The result is a pair (T, n) where T is the
    // token and n is the new index after skipping past T.
    ResultState get_token() {
        skip_whitespace();
        char c = curchar();
        if (eos()) {
            return ResultState(Token::EOS(), idx);
        } else if (c == '(') {
            return ResultState(Token::LParen(), idx + 1);
        } else if (c == ')') {
            return ResultState(Token::RParen(), idx + 1);
        } else if (c == '{') {
            return ResultState(Token::LBrace(), idx + 1);
        } else if (c == '}') {
            return ResultState(Token::RBrace(), idx + 1);
        } else if (c == '[') {
            return ResultState(Token::LBracket(), idx + 1);
        } else if (c == ']') {
            return ResultState(Token::RBracket(), idx + 1);
        } else if (c == '@') {
            return ResultState(Token::At(), idx + 1);
        } else if (c == '$') {
            return ResultState(Token::Dollar(), idx + 1);
        } else if (c == '#') {
            return ResultState(Token::Sharp(), idx + 1);
        } else if (c == ';') {
            return ResultState(Token::Semicolon(), idx + 1);
        } else if (c == '.' && nextchar() == '.') {
            return ResultState(Token::DoubleDot(), idx + 2);
        } else if (c == ',') {
            return ResultState(Token::Comma(), idx + 1);
        } else if (c == '=') {
            if (nextchar() == '=') {
                return ResultState(Token::DoubleEquals(), idx + 2);
            } else {
                return ResultState(Token::Equals(), idx + 1);
            }
        } else if (c == '!' && nextchar() == '=') {
            return ResultState(Token::NotEquals(), idx + 2);
        } else if (c == '<') {
            if (nextchar() == '=') {
                return ResultState(Token::LAngleEquals(), idx + 1);
            } else {
                return ResultState(Token::LAngle(), idx + 1);
            }
        } else if (c == '>') {
            if (nextchar() == '=') {
                return ResultState(Token::RAngleEquals(), idx + 2);
            } else {
                return ResultState(Token::RAngle(), idx + 1);
            }
        } else if (c == '+') {
            return ResultState(Token::Plus(), idx + 1);
        } else if (c == '-') {
            return ResultState(Token::Minus(), idx + 1);
        } else if (c == '*') {
            return ResultState(Token::Star(), idx + 1);
        } else if (c == '/') {
            return ResultState(Token::Slash(), idx + 1);
        } else if (c == '%') {
            return ResultState(Token::Percent(), idx + 1);
        } else if (c == '"') {
            return ResultState(Token::Quote(), idx + 1);
        } else if (is_digit(c)) {
            return read_number();
        } else {
            ResultState result = read_symbol();
            if (result.second == idx) {
                std::cerr << "Unhandled token character at " << position() << "\n";
                assert(false);
            }
            return result;
        }
    }

    // Read a multi-digit (and possibly fractional) number token.
    ResultState read_number() {
        char c = curchar();
        bool fractional = false;
        std::string snum;
        unsigned newidx = idx;
        while (is_digit(text[newidx])) {
            snum += text[newidx];
            newidx++;
        }
        if (text[newidx] == '.') {
            fractional = true;
            snum += ".";
            newidx++;
            while (is_digit(text[newidx])) {
                snum += text[newidx];
                newidx++;
            }
        }
        if (fractional) {
            return ResultState(Token::Fractional(snum), newidx);
        } else {
            return ResultState(Token::Int(snum), newidx);
        }
    }

    // Read a multi-character string of characters.
    ResultState read_symbol() {
        std::string sym = "";
        unsigned newidx = idx;
        while (is_alphanumeric(text[newidx])) {
            sym += text[newidx];
            newidx++;
        }
        return ResultState(get_multichar_token(sym), newidx);
    }

    // Return the correct token type for a string of letters. This
    // checks for reserved keywords.
    Token get_multichar_token(const std::string &s) {
        if (s.compare(Token::Return().value()) == 0) {
            return Token::Return();
        } else if (s.compare(Token::If().value()) == 0) {
            return Token::If();
        } else if (s.compare(Token::Else().value()) == 0) {
            return Token::Else();
        } else if (s.compare(Token::Def().value()) == 0) {
            return Token::Def();
        } else if (s.compare(Token::For().value()) == 0) {
            return Token::For();
        } else if (s.compare(Token::In().value()) == 0) {
            return Token::In();
        } else if (s.compare(Token::True().value()) == 0) {
            return Token::True();
        }  else if (s.compare(Token::False().value()) == 0) {
            return Token::False();
        } else {
            return Token::Symbol(s);
        }
    }
};

Parser::~Parser() {
    if (tokenizer) delete tokenizer;
}

// Return the entire contents of the file at the given path.
std::string Parser::read_file(const std::string &path) {
    std::ifstream t(path.c_str());
    if (!t.is_open()) {
        std::string msg = "Failed to open file at " + path;
        abort(msg);
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Parse the given file into Bish IR.
Module *Parser::parse(const std::string &path) {
    std::string contents = read_file(path);
    return parse_string(contents);
}

// Parse the given string into Bish IR.
Module *Parser::parse_string(const std::string &text) {
    if (tokenizer) delete tokenizer;

    // Insert a dummy block for root scope.
    std::string preprocessed = "{" + text + "}";
    tokenizer = new Tokenizer(preprocessed);

    Module *m = module();
    expect(tokenizer->peek(), Token::EOSType, "Expected end of string.");

    post_parse_passes(m);
    return m;
}

// Run an ordered list of postprocessing passes over the IR.
void Parser::post_parse_passes(Module *m) {
    // Construct IRNode hierarchy
    IRAncestorsPass ancestors;
    m->accept(&ancestors);

    // Type checking
    // TypeChecker types;
    // m->accept(&types);
}

// Assert that the given token is of the given type. If true, advance
// the tokenizer. If false, produce an error message.
void Parser::expect(const Token &t, Token::Type ty, const std::string &msg) {
    if (!t.isa(ty)) {
        std::stringstream errstr;
        errstr << "Parsing error: " << msg << " near " << tokenizer->position();
        abort(errstr.str());
    }
    tokenizer->next();
}

// Terminate the parsing process with the given error message.
void Parser::abort(const std::string &msg) {
    std::cerr << msg << "\n";
    exit(1);
}

// Return true if the given token is a unary operator.
bool Parser::is_unop_token(const Token &t) {
    return t.isa(Token::MinusType);
}

// Return true if the given token is a binary operator.
bool Parser::is_binop_token(const Token &t) {
    return t.isa(Token::PlusType) || t.isa(Token::MinusType) ||
        t.isa(Token::StarType) || t.isa(Token::SlashType) ||
        t.isa(Token::PercentType);
}

// Return the binary Operator corresponding to the given token.
BinOp::Operator Parser::get_binop_operator(const Token &t) {
    switch (t.type()) {
    case Token::DoubleEqualsType:
        return BinOp::Eq;
    case Token::NotEqualsType:
        return BinOp::NotEq;
    case Token::LAngleType:
        return BinOp::LT;
    case Token::LAngleEqualsType:
        return BinOp::LTE;
    case Token::RAngleType:
        return BinOp::GT;
    case Token::RAngleEqualsType:
        return BinOp::GTE;
    case Token::PlusType:
        return BinOp::Add;
    case Token::MinusType:
        return BinOp::Sub;
    case Token::StarType:
        return BinOp::Mul;
    case Token::SlashType:
        return BinOp::Div;
    case Token::PercentType:
        return BinOp::Mod;
    default:
        abort("Invalid operator for binary operation.");
        return BinOp::Add;
    }
}

// Return the unary Operator corresponding to the given token.
UnaryOp::Operator Parser::get_unaryop_operator(const Token &t) {
    switch (t.type()) {
    case Token::MinusType:
        return UnaryOp::Negate;
    default:
        abort("Invalid operator for unary operation.");
        return UnaryOp::Negate;
    }
}

// Return the Bish Type to represent the given IR node.
Type Parser::get_primitive_type(const IRNode *n) {
    if (const Integer *v = dynamic_cast<const Integer*>(n)) {
        return IntegerTy;
    } else if (const Fractional *v = dynamic_cast<const Fractional*>(n)) {
        return FractionalTy;
    } else if (const String *v = dynamic_cast<const String*>(n)) {
        return StringTy;
    } else if (const Boolean *v = dynamic_cast<const Boolean*>(n)) {
        return BooleanTy;
    } else {
        return UndefinedTy;
    }
}

void Parser::push_module(Module *m) {
    module_stack.push(m);
}

Module *Parser::pop_module() {
    Module *m = module_stack.top();
    module_stack.pop();
    return m;
}

void Parser::push_symbol_table(SymbolTable *s) {
    symbol_table_stack.push(s);
}

SymbolTable *Parser::pop_symbol_table() {
    SymbolTable *s = symbol_table_stack.top();
    symbol_table_stack.pop();
    return s;
}

Module *Parser::module() {
    Module *m = new Module();
    push_module(m);
    function_symbol_table = new SymbolTable();
    // The pre-main function has all module-level statements. If the
    // user also defined a main function, a call to pre_main will be
    // inserted as the first statement in the user main.
    Function *pre_main = new Function("main", block());
    m->set_main(pre_main);
    pop_module();
    delete function_symbol_table;
    return m;
}

// Parse a Bish block.
Block *Parser::block() {
    std::vector<IRNode *> statements;
    push_symbol_table(new SymbolTable());
    expect(tokenizer->peek(), Token::LBraceType, "Expected block to begin with '{'");
    do {
        while (tokenizer->peek().isa(Token::SharpType)) {
            tokenizer->scan_until('\n');
        }
        if (tokenizer->peek().isa(Token::RBraceType)) break;
        IRNode *s = stmt();
        if (s) statements.push_back(s);
    } while (!tokenizer->peek().isa(Token::RBraceType));
    expect(tokenizer->peek(), Token::RBraceType, "Expected block to end with '}'");
    Block *result = new Block(statements);
    delete pop_symbol_table();
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
    case Token::ReturnType:
        return returnstmt();
    case Token::IfType:
        return ifstmt();
    case Token::ForType:
        return forloop();
    case Token::DefType: {
        Function *f = functiondef();
        module_stack.top()->add_function(f);
        return NULL;
    }
    default:
        return otherstmt();
    }
}

IRNode *Parser::otherstmt() {
    std::string sym = symbol();
    IRNode *s = NULL;
    switch (tokenizer->peek().type()) {
    case Token::EqualsType:
        s = assignment(sym);
        break;
    case Token::LParenType:
        s = funcall(sym);
        break;
    default:
        abort("Unexpected token in statement.");
        s = NULL;
        break;
    }
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return s;
}

IRNode *Parser::externcall() {
    expect(tokenizer->peek(), Token::AtType, "Expected '@' to begin extern call.");
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    InterpolatedString *body = new InterpolatedString();
    do {
        std::string str = tokenizer->scan_until(Token::Dollar(), Token::RParen());
        body->push_str(str);
        if (tokenizer->peek().isa(Token::DollarType)) {
            tokenizer->next();
            if (tokenizer->peek().isa(Token::LParenType)) {
                tokenizer->next();
                str = tokenizer->scan_until(Token::RParen());
                tokenizer->next();
                body->push_str("$" + str);
            } else {
                Variable *v = var();
                body->push_var(v);
                body->push_str(tokenizer->scan_whitespace());
            }
        }
    } while (!tokenizer->peek().isa(Token::RParenType));
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    return new ExternCall(body);
}

IRNode *Parser::returnstmt() {
    expect(tokenizer->peek(), Token::ReturnType, "Expected return statement");
    IRNode *ret = expr();
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return new ReturnStatement(ret);
}

IRNode *Parser::ifstmt() {
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

IRNode *Parser::forloop() {
    expect(tokenizer->peek(), Token::ForType, "Expected for statement");
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    Variable *v = var();
    expect(tokenizer->peek(), Token::InType, "Expected keyword 'in'");
    IRNode *lower = atom(), *upper = NULL;
    if (tokenizer->peek().isa(Token::DoubleDotType)) {
        tokenizer->next();
        upper = atom();
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    IRNode *body = block();
    return new ForLoop(v, lower, upper, body);
}

Function *Parser::functiondef() {
    expect(tokenizer->peek(), Token::DefType, "Expected def statement");
    std::string name = symbol();
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    std::vector<Variable *> args;
    push_symbol_table(new SymbolTable());
    if (!tokenizer->peek().isa(Token::RParenType)) {
        args.push_back(arg());
        while (tokenizer->peek().isa(Token::CommaType)) {
            tokenizer->next();
            args.push_back(arg());
        }
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    Block *body = block();
    delete pop_symbol_table();
    Function *f = lookup_or_new_function(name);
    f->set_args(args);
    f->set_body(body);
    return f;
}

IRNode *Parser::funcall(const std::string &name) {
    expect(tokenizer->peek(), Token::LParenType, "Expected opening '('");
    std::vector<IRNode *> args;
    if (!tokenizer->peek().isa(Token::RParenType)) {
        args.push_back(expr());
        while (tokenizer->peek().isa(Token::CommaType)) {
            tokenizer->next();
            args.push_back(expr());
        }
    }
    expect(tokenizer->peek(), Token::RParenType, "Expected closing ')'");
    Function *f = lookup_or_new_function(name);
    return new FunctionCall(f, args);
}

IRNode *Parser::assignment(const std::string &name) {
    expect(tokenizer->peek(), Token::EqualsType, "Expected assignment operator");
    Variable *v = lookup_or_new_var(name);
    IRNode *e = expr();
    Type t = get_primitive_type(e);
    if (t != UndefinedTy) {
        symbol_table_stack.top()->insert(name, v, t);
    }
    return new Assignment(v, e);
}

Variable *Parser::var() {
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected variable to be a symbol");
    return lookup_or_new_var(name);//new Variable(name);
}

Variable *Parser::arg() {
    // Similar to var() but this always creates a new symbol.
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected argument to be a symbol");
    Variable *arg = new Variable(name);
    symbol_table_stack.top()->insert(name, arg, UndefinedTy);
    return arg;
}

IRNode *Parser::expr() {
    IRNode *a = relative();
    Token t = tokenizer->peek();
    if (t.isa(Token::DoubleEqualsType) || t.isa(Token::NotEqualsType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, relative());
        t = tokenizer->peek();
    }
    return a;
}

IRNode *Parser::relative() {
    IRNode *a = arith();
    Token t = tokenizer->peek();
    if (t.isa(Token::LAngleType) || t.isa(Token::LAngleEqualsType) ||
        t.isa(Token::RAngleType) || t.isa(Token::RAngleEqualsType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, arith());
        t = tokenizer->peek();
    }
    return a;
}

IRNode *Parser::arith() {
    IRNode *a = term();
    Token t = tokenizer->peek();
    while (t.isa(Token::PlusType) || t.isa(Token::MinusType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, term());
        t = tokenizer->peek();
    }
    return a;
}

IRNode *Parser::term() {
    IRNode *a = unary();
    Token t = tokenizer->peek();
    while (t.isa(Token::StarType) || t.isa(Token::SlashType) || t.isa(Token::PercentType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, unary());
        t = tokenizer->peek();
    }
    return a;
}

IRNode *Parser::unary() {
    Token t = tokenizer->peek();
    if (is_unop_token(t)) {
        tokenizer->next();
        return new UnaryOp(get_unaryop_operator(t), factor());
    } else {
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
            Variable *v = static_cast<Variable*>(a);
            // The symbol will be reinserted as a Function, not a Variable.
            remove_from_symbol_table(v->name);
            a = funcall(v->name);
        }
        return a;
    }
}

IRNode *Parser::atom() {
    Token t = tokenizer->peek();
    tokenizer->next();

    switch(t.type()) {
    case Token::SymbolType:
        return lookup_or_new_var(t.value());
    case Token::TrueType:
        return new Boolean(true);
    case Token::FalseType:
        return new Boolean(false);
    case Token::IntType:
        return new Integer(t.value());
    case Token::FractionalType:
        return new Fractional(t.value());
    case Token::QuoteType: {
        std::string str = tokenizer->scan_until(Token::Quote());
        expect(tokenizer->peek(), Token::QuoteType, "Unmatched '\"'");
        return new String(str);
    }
    default:
        abort("Invalid token type for atom.");
        return NULL;
    }
}

std::string Parser::symbol() {
    Token t = tokenizer->peek();
    expect(t, Token::SymbolType, "Expected symbol.");
    return t.value();
}

Variable *Parser::lookup_or_new_var(const std::string &name) {
    IRNode *result = lookup(name);
    if (result) {
        Variable *v = dynamic_cast<Variable*>(result);
        assert(v);
        return v;
    } else {
        Variable *v = new Variable(name);
        symbol_table_stack.top()->insert(name, v, UndefinedTy);
        return v;
    }
}

Function *Parser::lookup_or_new_function(const std::string &name) {
    SymbolTableEntry *e = function_symbol_table->lookup(name);
    Function *f = NULL;
    if (e) {
        f = dynamic_cast<Function*>(e->node);
    } else {
        f = new Function(name);
        function_symbol_table->insert(name, f, UndefinedTy);
    }
    assert(f);
    return f;
}
  
IRNode *Parser::lookup(const std::string &name) {
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
    return result;
}

void Parser::remove_from_symbol_table(const std::string &name) {
    symbol_table_stack.top()->remove(name);
}

}
