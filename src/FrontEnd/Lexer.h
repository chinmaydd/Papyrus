#ifndef PAPYRUS_FRONTEND_LEXER_H
#define PAPYRUS_FRONTEND_LEXER_H

#include "Papyrus/Logger/Logger.h"

#include <iostream>
#include <string>
#include <unordered_map>

namespace papyrus {
class Lexer {
public:
    enum Token {
        TOK_LETTER,
        TOK_DIGIT,

        TOK_RELOP_EQ,
        TOK_RELOP_NEQ,
        TOK_RELOP_LT,
        TOK_RELOP_LTE,
        TOK_RELOP_GT,
        TOK_RELOP_GTE,

        TOK_IDENT,
        TOK_NUM,

        TOK_LET,
        TOK_CALL,
        TOK_IF,
        TOK_THEN,
        TOK_ELSE,
        TOK_FI,
        TOK_WHILE,
        TOK_DO,
        TOK_OD,
        TOK_RETURN,

        TOK_VAR,
        TOK_ARRAY,
        TOK_FUNCTION,
        TOK_PROCEDURE,

        TOK_SEMICOLON,
        TOK_ROUND_OPEN,
        TOK_ROUND_CLOSED,
        TOK_CURLY_OPEN,
        TOK_CURLY_CLOSED,
        TOK_SQUARE_OPEN,
        TOK_SQUARE_CLOSED,
        TOK_COMMA,
        TOK_LEFTARROW,
        TOK_DOT,

        TOK_BINOP_MUL,
        TOK_BINOP_ADD,
        TOK_BINOP_SUB,
        TOK_BINOP_DIV,

        TOK_MAIN,

        // XXX: Currently adding this as a safety measure
        TOK_ANY,
        TOK_NONE,
        TOK_EOF,
    };

    Lexer(std::istream &i_buf);

    Token GetNextToken();
    const std::string& GetBuffer() const { return current_buffer_; };
    const Token GetToken() const { return current_token_; };
    const int GetLineNo() const { return current_lineno_;}
    const std::string& GetTokenTranslation(Token& t) const { return token_translations_.at(t); }

private:
    // Variables
    std::istream &input_buffer_;
    std::string current_buffer_;
    int current_lineno_;
    char ch_;
    char last_char_;
    Token current_token_;
    // Map for Reserved words in the language
    std::unordered_map<std::string, Token> reserved_words_;
    structlog LOGCFG_;

    // Methods
    char ReadChar();
    char PeekChar();
    void ReadAndAdvance();
    void ConsumeLine();
    void ConsumeWhitespaceAndComments();
    inline void Reserve(std::string&, Token&);

    std::unordered_map<Token, std::string> token_translations_ {
        {TOK_LETTER, "TOK_LETTER"},
        {TOK_DIGIT, "TOK_DIGIT"},

        {TOK_RELOP_EQ, "TOK_RELOP_EQ"},
        {TOK_RELOP_NEQ, "TOK_RELOP_NEQ"},
        {TOK_RELOP_LT, "TOK_RELOP_LT"},
        {TOK_RELOP_LTE, "TOK_RELOP_LTE"},
        {TOK_RELOP_GT, "TOK_RELOP_GT"},
        {TOK_RELOP_GTE, "TOK_RELOP_GTE"},

        {TOK_IDENT, "TOK_IDENT"},
        {TOK_NUM, "TOK_NUM"},

        {TOK_LET, "TOK_LET"},
        {TOK_CALL, "TOK_CALL"},
        {TOK_IF, "TOK_IF"},
        {TOK_THEN, "TOK_THEN"},
        {TOK_ELSE, "TOK_ELSE"},
        {TOK_FI, "TOK_FI"},
        {TOK_WHILE, "TOK_WHILE"},
        {TOK_DO, "TOK_DO"},
        {TOK_OD, "TOK_OD"},
        {TOK_RETURN, "TOK_RETURN"},

        {TOK_VAR, "TOK_VAR"},
        {TOK_ARRAY, "TOK_ARRAY"},
        {TOK_FUNCTION, "TOK_FUNCTION"},
        {TOK_PROCEDURE, "TOK_PROCEDURE"},

        {TOK_SEMICOLON, "TOK_SEMICOLON"},
        {TOK_ROUND_OPEN, "TOK_ROUND_OPEN"},
        {TOK_ROUND_CLOSED, "TOK_ROUND_CLOSED"},
        {TOK_CURLY_OPEN, "TOK_CURLY_OPEN"},
        {TOK_CURLY_CLOSED, "TOK_CURLY_CLOSED"},
        {TOK_SQUARE_OPEN, "TOK_SQUARE_OPEN"},
        {TOK_SQUARE_CLOSED, "TOK_SQUARE_CLOSED"},
        {TOK_COMMA, "TOK_COMMA"},
        {TOK_LEFTARROW, "TOK_LEFTARROW"},
        {TOK_DOT, "TOK_DOT"},

        {TOK_BINOP_MUL, "TOK_BINOP_MUL"},
        {TOK_BINOP_ADD, "TOK_BINOP_ADD"},
        {TOK_BINOP_SUB, "TOK_BINOP_SUB"},
        {TOK_BINOP_DIV, "TOK_BINOP_DIV"},

        {TOK_MAIN, "TOK_MAIN"},

        {TOK_ANY, "TOK_ANY"},
        {TOK_NONE, "TOK_NONE"},
        {TOK_EOF, "TOK_EOF"},
    };
};

} // end namespace papyrus

#endif
