#ifndef PAPYRUS_FRONTEND_LEXER_H
#define PAPYRUS_FRONTEND_LEXER_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"

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
        TOK_VARORARR,
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
    const std::string& GetBuffer() const { 
        return current_buffer_;
    }
    Token GetToken() const { 
        return current_token_;
    }
    int GetLineNo() const { 
        return current_lineno_;
    }
    long int ConvertBufferToNumber() const { 
        return std::stol(current_buffer_);
    }
    const std::string& GetTokenTranslation(const Token& tok) const { 
        return token_translations_.at(tok);
    }
    bool IsRelationalOp(const Token&) const;
    RelationalOperator GetOperatorForToken(const Token&) const;

private:
    // Variables
    std::istream& input_buffer_;
    std::string current_buffer_;
    int current_lineno_;
    char ch_;
    char last_char_;
    Token current_token_;

    // Map for Reserved words in the language
    std::unordered_map<std::string, Token> reserved_words_;
    structlog LOGCFG_;

    // Methods
    // TODO: Add const, inline qualifiers
    char ReadChar();
    char PeekChar();
    void ReadAndAdvance();
    void ConsumeLine();
    void ConsumeWhitespaceAndComments();
    inline void Reserve(std::string&, Token&);

    std::unordered_map<Token, std::string> token_translations_ {
        {TOK_LETTER, "LETTER"},
        {TOK_DIGIT, "DIGIT"},

        {TOK_RELOP_EQ, "RELOP_EQ"},
        {TOK_RELOP_NEQ, "RELOP_NEQ"},
        {TOK_RELOP_LT, "RELOP_LT"},
        {TOK_RELOP_LTE, "RELOP_LTE"},
        {TOK_RELOP_GT, "RELOP_GT"},
        {TOK_RELOP_GTE, "RELOP_GTE"},

        {TOK_IDENT, "IDENT"},
        {TOK_NUM, "NUM"},

        {TOK_LET, "LET"},
        {TOK_CALL, "CALL"},
        {TOK_IF, "IF"},
        {TOK_THEN, "THEN"},
        {TOK_ELSE, "ELSE"},
        {TOK_FI, "FI"},
        {TOK_WHILE, "WHILE"},
        {TOK_DO, "DO"},
        {TOK_OD, "OD"},
        {TOK_RETURN, "RETURN"},

        {TOK_VAR, "VAR"},
        {TOK_ARRAY, "ARRAY"},
        {TOK_VARORARR, "VAR OR ARRAY"},
        {TOK_FUNCTION, "FUNCTION"},
        {TOK_PROCEDURE, "PROCEDURE"},

        {TOK_SEMICOLON, "SEMICOLON"},
        {TOK_ROUND_OPEN, "ROUND_OPEN"},
        {TOK_ROUND_CLOSED, "ROUND_CLOSED"},
        {TOK_CURLY_OPEN, "CURLY_OPEN"},
        {TOK_CURLY_CLOSED, "CURLY_CLOSED"},
        {TOK_SQUARE_OPEN, "SQUARE_OPEN"},
        {TOK_SQUARE_CLOSED, "SQUARE_CLOSED"},
        {TOK_COMMA, "COMMA"},
        {TOK_LEFTARROW, "LEFTARROW"},
        {TOK_DOT, "DOT"},

        {TOK_BINOP_MUL, "BINOP_MUL"},
        {TOK_BINOP_ADD, "BINOP_ADD"},
        {TOK_BINOP_SUB, "BINOP_SUB"},
        {TOK_BINOP_DIV, "BINOP_DIV"},

        {TOK_MAIN, "MAIN"},

        {TOK_ANY, "ANY"},
        {TOK_NONE, "NONE"},
        {TOK_EOF, "EOF"},
    };
};
} // namespace papyrus

#endif /* PAPYRUS_FRONTEND_LEXER_H */
