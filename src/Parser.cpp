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
    block();
    return new AST(current_ast_node->root());
}

void Parser::expect(const Token &t, Token::Type ty, const std::string &msg) {
    if (!t.isa(ty)) {
        std::cerr << "Parsing error: " << msg << " near " << tokenizer->position() << "\n";
        abort();
    }
    tokenizer->next();
}

void Parser::abort() {
    exit(1);
}

bool Parser::is_unop_token(const Token &t) {
    return t.isa(Token::MinusType);
}

bool Parser::is_binop_token(const Token &t) {
    return t.isa(Token::PlusType) || t.isa(Token::MinusType) ||
        t.isa(Token::StarType) || t.isa(Token::SlashType);
}

void Parser::block() {
    Token t = tokenizer->peek();
    expect(t, Token::LBraceType, "Expected block to begin with '{'");
    current_ast_node = new Block(current_ast_node);
    do {
        stmt();
    } while (!tokenizer->peek().isa(Token::RBraceType));
    expect(tokenizer->peek(), Token::RBraceType, "Expected block to end with '}'");
    current_ast_node = current_ast_node->parent();
}

void Parser::stmt() {
    Token t = tokenizer->peek();
    var();
    expect(tokenizer->peek(), Token::EqualsType, "Expected assignment operator");
    expr();
    expect(tokenizer->peek(), Token::SemicolonType, "Expected statement to end with ';'");
}

void Parser::var() {
    expect(tokenizer->peek(), Token::SymbolType, "Expected variable to be a symbol");
}

void Parser::expr() {
    Token t = tokenizer->peek();
    if (t.isa(Token::LParenType)) {
        tokenizer->next();
        expr();
        expect(tokenizer->peek(), Token::RParenType, "Unmatched '('");
    } else if (is_unop_token(t)) {
        unop();
    } else {
        atom();
        Token t = tokenizer->peek();
        while (is_binop_token(t)) {
            tokenizer->next();
            atom();
            t = tokenizer->peek();
        }
    }
}

void Parser::binop() {
    tokenizer->next();
}

void Parser::unop() {
    tokenizer->next();
}

void Parser::atom() {
    tokenizer->next();
}

}
