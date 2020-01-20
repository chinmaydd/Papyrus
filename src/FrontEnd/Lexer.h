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

    Token get_next_token();
    const std::string& get_buffer() const { return current_buffer; };
    const Token get_token() const { return current_token; };

private:
    // Variables
    std::istream &input_buffer;
    std::string current_buffer;
    int current_lineno;
    char ch;
    char last_char;
    Token current_token;
    // Map for reserved words in the language
    std::unordered_map<std::string, Token> reserved_words;
    structlog LOGCFG;

    // Methods
    char read_ch();
    char peek_ch();
    void read_and_advance();
    void consume_line();
    void consume_whitespace_and_comments();
    inline void reserve(std::string&, Token&);
};
}

#endif
