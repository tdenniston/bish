#include <cassert>
#include <cstdlib>
#include <sstream>
#include <iostream>

#include "Parser.h"

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


class Tokenizer {
public:
    Tokenizer(const std::string &t) : text(t), idx(0), lineno(1) {}

    Token peek() {
        ResultState st = get_token();
        return st.first;
    }

    void next() {
        ResultState st = get_token();
        idx = st.second;
    }

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

    inline char curchar() const {
        return text[idx];
    }

    inline char nextchar() const {
        return text[idx + 1];
    }

    inline bool eos() const {
        return idx >= text.length();
    }

    inline void skip_whitespace() {
        while (!eos() && is_whitespace(curchar())) {
            if (is_newline(curchar())) ++lineno;
            ++idx;
        }
    }

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
        } else if (is_digit(c) || (c == '-' && is_digit(nextchar()))) {
            return read_number();
        } else {
            return read_symbol();
        }
    }

    ResultState read_number() {
        char c = curchar();
        bool fractional = false;
        std::string snum;
        unsigned newidx;
        if (c == '-') {
            assert(false);
            snum += "-";
            newidx = idx + 1;
        } else {
            newidx = idx;
        }
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
    if (current_ast_node) delete current_ast_node;
}

AST *Parser::parse(const std::string &text) {
    if (tokenizer) delete tokenizer;
    if (current_ast_node) delete current_ast_node;
    tokenizer = new Tokenizer(text);
    ASTNode *b = block();
    return new AST(b);
}

void Parser::expect(const Token &t, Token::Type ty, const std::string &msg) {
    if (!t.isa(ty)) {
        std::stringstream errstr;
        errstr << "Parsing error: " << msg << " near " << tokenizer->position();
        abort(errstr.str());
    }
    tokenizer->next();
}

void Parser::abort(const std::string &msg) {
    std::cerr << msg << "\n";
    exit(1);
}

bool Parser::is_unop_token(const Token &t) {
    return t.isa(Token::MinusType);
}

bool Parser::is_binop_token(const Token &t) {
    return t.isa(Token::PlusType) || t.isa(Token::MinusType) ||
        t.isa(Token::StarType) || t.isa(Token::SlashType);
}

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

UnaryOp::Operator Parser::get_unaryop_operator(const Token &t) {
    switch (t.type()) {
    case Token::MinusType:
        return UnaryOp::Negate;
    default:
        abort("Invalid operator for unary operation.");
        return UnaryOp::Negate;
    }
}

Block *Parser::block() {
    std::vector<ASTNode *> statements;
    Token t = tokenizer->peek();
    expect(t, Token::LBraceType, "Expected block to begin with '{'");
    do {
        statements.push_back(stmt());
    } while (!tokenizer->peek().isa(Token::RBraceType));
    expect(tokenizer->peek(), Token::RBraceType, "Expected block to end with '}'");
    return new Block(statements);
}

ASTNode *Parser::stmt() {
    Variable *v = var();
    expect(tokenizer->peek(), Token::EqualsType, "Expected assignment operator");
    ASTNode *e = expr();
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
    return new Assignment(v, e);
}

Variable *Parser::var() {
    std::string name = tokenizer->peek().value();
    expect(tokenizer->peek(), Token::SymbolType, "Expected variable to be a symbol");
    return new Variable(name);
}

ASTNode *Parser::expr() {
    Token t = tokenizer->peek();
    if (t.isa(Token::LParenType)) {
        tokenizer->next();
        ASTNode *e = expr();
        expect(tokenizer->peek(), Token::RParenType, "Unmatched '('");
        return e;
    } else if (is_unop_token(t)) {
        return unop();
    } else {
        ASTNode *a = atom();
        if (is_binop_token(tokenizer->peek())) {
            return binop(a);
        }
        return a;
    }
}

BinOp *Parser::binop(ASTNode *a) {
    BinOp *bin = NULL;
    while (is_binop_token(tokenizer->peek())) {
        BinOp::Operator op = get_binop_operator(tokenizer->peek());
        tokenizer->next();
        ASTNode *b = atom();
        bin = new BinOp(op, a, b);
        a = bin;
    }
    return bin;
}

UnaryOp *Parser::unop() {
    UnaryOp::Operator op = get_unaryop_operator(tokenizer->peek());
    tokenizer->next();
    ASTNode *a = atom();
    return new UnaryOp(op, a);
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
    default:
        abort("Invalid token type for atom.");
        return NULL;
    }
}

}
