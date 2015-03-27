#ifndef __BISH_TOKENIZER_H__
#define __BISH_TOKENIZER_H__

#include <set>
#include <stack>
#include <string>
#include <vector>
#include "IR.h"

namespace Bish {

class Token {
public:
    typedef enum { AndType,
                   AtType,
                   BackslashType,
                   BreakType,
                   CommaType,
                   ContinueType,
                   DefType,
                   DollarType,
                   DotType,
                   DoubleDotType,
                   DoubleEqualsType,
                   EOSType,
                   ElseType,
                   EqualsType,
                   FalseType,
                   ForType,
                   FractionalType,
                   IfType,
                   ImportType,
                   InType,
                   IntType,
                   LAngleEqualsType,
                   RAngleEqualsType,
                   LAngleType,
                   RAngleType,
                   LBraceType,
                   RBraceType,
                   LBracketType,
                   RBracketType,
                   LParenType,
                   RParenType,
                   MinusType,
                   NotEqualsType,
                   NotType,
                   OrType,
                   PercentType,
                   PipeType,
                   PlusType,
                   QuoteType,
                   ReturnType,
                   SemicolonType,
                   SharpType,
                   SlashType,
                   StarType,
                   SymbolType,
                   TrueType,
                   UnderscoreType,
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

    static Token Underscore() {
        return Token(UnderscoreType, "_");
    }

    static Token At() {
        return Token(AtType, "@");
    }

    static Token Pipe() {
        return Token(PipeType, "|");
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

    static Token Import() {
        return Token(ImportType, "import");
    }

    static Token Return() {
        return Token(ReturnType, "return");
    }

    static Token Break() {
      return Token(BreakType, "break");
    }

    static Token Continue() {
      return Token(ContinueType, "continue");
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

    static Token And() {
        return Token(AndType, "and");
    }

    static Token Or() {
        return Token(OrType, "or");
    }

    static Token Not() {
        return Token(NotType, "not");
    }

    static Token Dot() {
        return Token(DotType, ".");
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

    static Token Backslash() {
        return Token(BackslashType, "\\");
    }

    static Token Percent() {
        return Token(PercentType, "%");
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

/*
 * The Bish tokenizer. Given a string to tokenize, use the peek() and
 * next() methods to produce a stream of tokens.
 */
class Tokenizer {
public:
    Tokenizer(const std::string &p, const std::string &t) : path(p), text(t), idx(0), lineno(0) {}

    // Return the token at the head of the stream, but do not skip it.
    Token peek();
    // Skip the token currently at the head of the stream.
    void next();
    // Return the substring beginning at the current index and
    // continuing until the first occurrence of one of the given
    // tokens. Any of the given tokens prefixed by '\' are ignored
    // during scanning. If the keep_literal_backslash parameter is
    // true, '\' characters are kept in the output.  E.g. scanning for
    // '(' in the string "test\))" would return "test\)" with
    // keep_literal_backslash set to true, and "test)" with
    // keep_literal_backslash set to false.
    std::string scan_until(const std::vector<Token> &tokens, bool keep_literal_backslash);
    // Return the substring beginning at the current index and
    // continuing until the first occurrence of a character of the
    // given value.
    std::string scan_until(char c);
    // Return a human-readable representation of the current position
    // in the string.
    std::string position() const;
    // Start a debug record.
    void start_debug_info();
    // Finish a debug record and return it.
    IRDebugInfo end_debug_info();
private:
    typedef std::pair<Token, unsigned> ResultState;
    std::stack<IRDebugInfo> debug_info_stack;
    const std::string path;
    const std::string &text;
    unsigned idx;
    unsigned lineno;

    // Return the current character.
    inline char curchar() const;
    // Return the next character
    inline char nextchar() const;
    // Return the string from the current character to n characters ahead.
    inline std::string lookahead(int n) const;
    // Return true if the tokenizer is at "end of string".
    inline bool eos() const;
    // Skip ahead until the next non-whitespace character.
    inline void skip_whitespace();
    // Form the next token. The result is a pair (T, n) where T is the
    // token and n is the new index after skipping past T.
    ResultState get_token();
    // Read a multi-digit (and possibly fractional) number token.
    ResultState read_number();
    // Read a multi-character string of characters.
    ResultState read_symbol();
    // Return the correct token type for a string of letters. This
    // checks for reserved keywords.
    Token get_multichar_token(const std::string &s);
};

bool is_unop_token(const Token &t);
bool is_binop_token(const Token &t);
BinOp::Operator get_binop_operator(const Token &t);
UnaryOp::Operator get_unaryop_operator(const Token &t);
IORedirection::Operator get_redirection_operator(const Token &t);

}

#endif
