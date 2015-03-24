#include <cassert>
#include <iostream>
#include <sstream>
#include "Tokenizer.h"

using namespace Bish;

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

// Return true if c is a valid character for a symbol (e.g. variable or function name);
inline bool is_symbol_char(char c) {
    return is_alphanumeric(c) || c == '_';
}
} // end anonymous namespace

// Return the token at the head of the stream, but do not skip it.
Token Tokenizer::peek() {
    ResultState st = get_token();
    return st.first;
}

// Skip the token currently at the head of the stream.
void Tokenizer::next() {
    ResultState st = get_token();
    idx = st.second;
}

// Return the substring beginning at the current index and
// continuing until the first occurrence of one of the given
// tokens. Any of the given tokens prefixed by '\' are ignored
// during scanning. If the keep_literal_backslash parameter is
// true, '\' characters are kept in the output.  E.g. scanning for
// '(' in the string "test\))" would return "test\)" with
// keep_literal_backslash set to true, and "test)" with
// keep_literal_backslash set to false.
std::string Tokenizer::scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash) {
    bool found = false, escape = false;
    std::string result;
    const unsigned n = tokens.size();
    std::set<char> firstchars;
    std::vector<unsigned> lengths;
    for (std::vector<Token>::const_iterator I = tokens.begin(), E = tokens.end(); I != E; ++I) {
        firstchars.insert((*I).value()[0]);
        lengths.push_back((*I).value().size());
    }

    while (!eos() && !found) {
        unsigned i = 0;
        while (!eos() && (firstchars.find(curchar()) == firstchars.end() || escape)) {
            if (curchar() == '\\') {
                escape = !escape;
                if (keep_literal_backslash) result += curchar();
            } else {
                escape = false;
                result += curchar();
            }
            idx++;
        }
        if (eos()) break;
        for (i = 0; i < n; i++) {
            const std::string &s = tokens[i].value();
            unsigned len = lengths[i];
            if (s.compare(lookahead(len)) == 0) {
                found = true;
                idx += len - 1;
                break;
            }
        }
    }
    return result;
}

// Return the substring beginning at the current index and
// continuing until the first occurrence of a character of the
// given value.
std::string Tokenizer::scan_until(char c) {
    unsigned start = idx;
    while (curchar() != c) {
        idx++;
    }
    return text.substr(start, idx - start);
}

// Return a human-readable representation of the current position
// in the string.
std::string Tokenizer::position() const {
    std::stringstream s;
    s << "character '" << text[idx] << "', line " << lineno;
    return s.str();
}

// Return the current character.
inline char Tokenizer::curchar() const {
    return text[idx];
}

// Return the next character
inline char Tokenizer::nextchar() const {
    return text[idx + 1];
}

inline std::string Tokenizer::lookahead(int n) const {
    if (idx + n >= text.length()) return "";
    return text.substr(idx, n);
}

// Return true if the tokenizer is at "end of string".
inline bool Tokenizer::eos() const {
    return idx >= text.length();
}

// Skip ahead until the next non-whitespace character.
inline void Tokenizer::skip_whitespace() {
    while (!eos() && is_whitespace(curchar())) {
        if (is_newline(curchar())) ++lineno;
        ++idx;
    }
}

// Form the next token. The result is a pair (T, n) where T is the
// token and n is the new index after skipping past T.
Tokenizer::ResultState Tokenizer::get_token() {
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
    } else if (c == '|') {
        return ResultState(Token::Pipe(), idx + 1);
    } else if (c == '$') {
        return ResultState(Token::Dollar(), idx + 1);
    } else if (c == '#') {
        return ResultState(Token::Sharp(), idx + 1);
    } else if (c == ';') {
        return ResultState(Token::Semicolon(), idx + 1);
    } else if (c == '.') {
        if (nextchar() == '.') {
            return ResultState(Token::DoubleDot(), idx + 2);
        } else {
            return ResultState(Token::Dot(), idx + 1);
        }
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
            return ResultState(Token::LAngleEquals(), idx + 2);
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
    } else if (c == '\\') {
        return ResultState(Token::Backslash(), idx + 1);
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
Tokenizer::ResultState Tokenizer::read_number() {
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
Tokenizer::ResultState Tokenizer::read_symbol() {
    std::string sym = "";
    unsigned newidx = idx;
    while (is_symbol_char(text[newidx])) {
        sym += text[newidx];
        newidx++;
    }
    return ResultState(get_multichar_token(sym), newidx);
}

// Return the correct token type for a string of letters. This
// checks for reserved keywords.
Token Tokenizer::get_multichar_token(const std::string &s) {
    if (s.compare(Token::Return().value()) == 0) {
        return Token::Return();
    } else if (s.compare(Token::Import().value()) == 0) {
        return Token::Import();
    } else if (s.compare(Token::Break().value()) == 0) {
        return Token::Break();
    } else if (s.compare(Token::Continue().value()) == 0) {
        return Token::Continue();
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
    } else if (s.compare(Token::And().value()) == 0) {
        return Token::And();
    } else if (s.compare(Token::Or().value()) == 0) {
        return Token::Or();
    } else if (s.compare(Token::Not().value()) == 0) {
        return Token::Not();
    } else if (s.compare(Token::True().value()) == 0) {
        return Token::True();
    }  else if (s.compare(Token::False().value()) == 0) {
        return Token::False();
    } else {
        return Token::Symbol(s);
    }
}

