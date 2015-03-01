#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Parser.h"
#include "TypeChecker.h"

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

inline bool is_paren(char c) {
    return c == '(' || c == ')';
}

inline bool is_brace(char c) {
    return c == '{' || c == '}';
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
    // 'type'.
    std::string scan_until(Token::Type type) {
        unsigned start = idx;
        Token t = peek();
        while (!t.isa(type) && !eos()) {
            next();
            t = peek();
        }
        unsigned len = idx - start;
        return text.substr(start, len);
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
        } else if (c == ';') {
            return ResultState(Token::Semicolon(), idx + 1);
        } else if (c == '=') {
            return ResultState(Token::Equals(), idx + 1);
        } else if (c == '+') {
            return ResultState(Token::Plus(), idx + 1);
        } else if (c == '-') {
            return ResultState(Token::Minus(), idx + 1);
        } else if (c == '*') {
            return ResultState(Token::Star(), idx + 1);
        } else if (c == '/') {
            return ResultState(Token::Slash(), idx + 1);
        } else if (c == '"') {
            return ResultState(Token::Quote(), idx + 1);
        } else if (is_digit(c)) {
            return read_number();
        } else {
            return read_symbol();
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
        return ResultState(Token::Symbol(sym), newidx);
    }
};

Parser::~Parser() {
    if (tokenizer) delete tokenizer;
}

// Return the entire contents of the file at the given path.
std::string Parser::read_file(const std::string &path) {
    std::ifstream t(path);
    if (!t.is_open()) {
        std::string msg = "Failed to open file at " + path;
        abort(msg);
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Parse the given file into a Bish AST.
AST *Parser::parse(const std::string &path) {
    std::string contents = read_file(path);
    return parse_string(contents);
}

// Parse the given string into a Bish AST.
AST *Parser::parse_string(const std::string &text) {
    if (tokenizer) delete tokenizer;

    // Insert a dummy block for root scope.
    std::string preprocessed = "{" + text + "}";
    tokenizer = new Tokenizer(preprocessed);
    
    AST *ast = new AST(block());
    expect(tokenizer->peek(), Token::EOSType, "Expected end of string.");
    // Type checking
    TypeChecker types;
    ast->accept(&types);
    return ast;
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
        t.isa(Token::StarType) || t.isa(Token::SlashType);
}

// Return the binary Operator corresponding to the given token.
BinOp::Operator Parser::get_binop_operator(const Token &t) {
    switch (t.type()) {
    case Token::PlusType:
        return BinOp::Add;
    case Token::MinusType:
        return BinOp::Sub;
    case Token::StarType:
        return BinOp::Mul;
    case Token::SlashType:
        return BinOp::Div;
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

// Return the Bish Type to represent the given AST node.
Type Parser::get_primitive_type(const ASTNode *n) {
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

// Parse a Bish block.
Block *Parser::block() {
    SymbolTable *old = current_symbol_table;
    current_symbol_table = new SymbolTable(old);
    std::vector<ASTNode *> statements;
    Token t = tokenizer->peek();
    expect(t, Token::LBraceType, "Expected block to begin with '{'");
    do {
        statements.push_back(stmt());
    } while (!tokenizer->peek().isa(Token::RBraceType));
    expect(tokenizer->peek(), Token::RBraceType, "Expected block to end with '}'");
    Block *result = new Block(statements, current_symbol_table);
    current_symbol_table = old;
    return result;
}

ASTNode *Parser::stmt() {
    Token t = tokenizer->peek();
    switch (t.type()) {
    case Token::LBraceType:
        return block();
    default:
        ASTNode *a = assignment();
        expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
        return a;
    }
}

ASTNode *Parser::assignment() {
    Variable *v = var();
    expect(tokenizer->peek(), Token::EqualsType, "Expected assignment operator.");
    ASTNode *e = expr();
    Type t = get_primitive_type(e);
    if (t != UndefinedTy) {
        current_symbol_table->insert(v->name, t);
    }
    return new Assignment(v, e);
}

Variable *Parser::var() {
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected variable to be a symbol");
    return new Variable(name);
}

ASTNode *Parser::expr() {
    ASTNode *a = term();
    Token t = tokenizer->peek();
    while (t.isa(Token::PlusType) || t.isa(Token::MinusType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, term());
        t = tokenizer->peek();
    }
    return a;
}

ASTNode *Parser::term() {
    ASTNode *a = unary();
    Token t = tokenizer->peek();
    while (t.isa(Token::StarType) || t.isa(Token::SlashType)) {
        tokenizer->next();
        a = new BinOp(get_binop_operator(t), a, unary());
        t = tokenizer->peek();
    }
    return a;
}

ASTNode *Parser::unary() {
    Token t = tokenizer->peek();
    if (is_unop_token(t)) {
        tokenizer->next();
        return new UnaryOp(get_unaryop_operator(t), factor());
    } else {
        return factor();
    }
}

ASTNode *Parser::factor() {
    if (tokenizer->peek().isa(Token::LParenType)) {
        tokenizer->next();
        ASTNode *e = expr();
        expect(tokenizer->peek(), Token::RParenType, "Unmatched '('");
        return e;
    } else {
        return atom();
    }
}

ASTNode *Parser::atom() {
    Token t = tokenizer->peek();
    tokenizer->next();
    
    switch(t.type()) {
    case Token::SymbolType:
        return new Variable(t.value());
    case Token::IntType:
        return new Integer(t.value());
    case Token::FractionalType:
        return new Fractional(t.value());
    case Token::QuoteType: {
        std::string str = tokenizer->scan_until(Token::QuoteType);
        expect(tokenizer->peek(), Token::QuoteType, "Unmatched '\"'");
        return new String(str);
    }
    default:
        abort("Invalid token type for atom.");
        return NULL;
    }
}

}
