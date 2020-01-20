#include "Lexer.h"

using namespace papyrus;

#define RESERVE_WORD(a, b) reserved_words.insert({a,b})

char Lexer::read_ch() {
    last_char = ch;
    input_buffer.get(ch); 

    if (input_buffer.eof()) return EOF;
    return ch;
}

char Lexer::peek_ch() {
    if (input_buffer.eof()) return EOF;

    return input_buffer.peek();
}

void Lexer::read_and_advance() {
    current_buffer.push_back(read_ch());
}

void Lexer::consume_line() {
    char peek_char = peek_ch();
    while (peek_char != EOF && peek_char != '\n') {
        read_ch();
        peek_char = peek_ch();
    }
}

void Lexer::consume_whitespace_and_comments() {
    char peek_char = peek_ch();
    while (peek_char != EOF) {
        if (peek_char == ' ' || peek_char == '\t' || peek_char == '\r') {
            read_ch();
        } else if (peek_char == '\n') {
            current_lineno += 1;
            read_ch();
        } else if (peek_char == '#') {
            consume_line();
        } else if (peek_ch() == '/') {
            read_ch();
            if (peek_ch() == '/')
                consume_line();
            else {
                LOG(ERROR) << "[LEXER] Invalid character found after '/' on Line: " << current_lineno;
            }
        } else
            break;

        peek_char = peek_ch();
    }
}

Lexer::Lexer(std::istream &i_buf) : 
    input_buffer(i_buf),
    current_buffer(),
    current_lineno(1),
    ch('\0'),
    last_char(' '),
    current_token(Lexer::TOK_NONE),
    reserved_words(),
    LOGCFG() {
        RESERVE_WORD("let", TOK_LET);
        RESERVE_WORD("call", TOK_CALL);
        RESERVE_WORD("if", TOK_IF);
        RESERVE_WORD("then", TOK_THEN);
        RESERVE_WORD("else", TOK_ELSE);
        RESERVE_WORD("while", TOK_WHILE);
        RESERVE_WORD("do", TOK_DO);
        RESERVE_WORD("od", TOK_OD);
        RESERVE_WORD("return", TOK_RETURN);
        RESERVE_WORD("var", TOK_VAR);
        RESERVE_WORD("array", TOK_ARRAY);
        RESERVE_WORD("function", TOK_FUNCTION);
        RESERVE_WORD("procedure", TOK_PROCEDURE);
}
    

void Lexer::reserve(std::string& word, Lexer::Token& token) {
    RESERVE_WORD(word, token);
}

Lexer::Token Lexer::get_next_token() {
    current_buffer = "";
    current_token = Token::TOK_NONE;

    consume_whitespace_and_comments();
        
    if (std::isalpha(peek_ch())) {
        read_and_advance();
        while (std::isalnum(peek_ch())) {
            read_and_advance();
        }
        auto res_word_it = reserved_words.find(current_buffer);
        if (res_word_it != reserved_words.end()) {
            // Found a reserved word!
            // Return token since we have already stored it.
            current_token = res_word_it->second;
        } else {
            current_token = TOK_IDENT;
        }
    } else if (std::isdigit(peek_ch())) {
        read_and_advance();
        while (std::isdigit(peek_ch())) {
            read_and_advance();
        }
        if (std::isalpha(peek_ch())) {
            LOG(ERROR) << "[LEXER] Invalid number literal found on Line: " << current_lineno;
        } else {
            current_token = TOK_NUM;
        }
    } else {
        switch(peek_ch()) {
            case '!':
            case '=':
                read_and_advance();
                if (peek_ch() == '=') {
                    read_and_advance();
                    current_token = last_char == '!' ? TOK_RELOP_NEQ : TOK_RELOP_EQ;
                } else {
                    LOG(ERROR) << "[LEXER] Invalid character found after " << last_char << " on Line: " << current_lineno;
                }
                break;
            case '<':
                read_and_advance();
                if (peek_ch() == '-') {
                    read_and_advance();
                    current_token = TOK_LEFTARROW;
                } else {
                    if (peek_ch() == '=') {
                        read_and_advance();
                        current_token = TOK_RELOP_LTE;
                    } else {
                        read_and_advance();
                        current_token = TOK_RELOP_LT;
                    }
                }
                break;
            case '>':
                read_and_advance();
                if (peek_ch() == '=') {
                    read_and_advance();
                    current_token = TOK_RELOP_GTE;
                } else {
                    read_and_advance();
                    current_token = TOK_RELOP_GT;
                }
                break;
            case ';':
                read_and_advance();
                current_token = TOK_SEMICOLON;
                break;
            case ',':
                read_and_advance();
                current_token = TOK_COMMA;
                break;
            case '(':
                read_and_advance();
                current_token = TOK_ROUND_OPEN;
                break;
            case ')':
                read_and_advance();
                current_token = TOK_ROUND_CLOSED;
                break;
            case '{':
                read_and_advance();
                current_token = TOK_CURLY_OPEN;
                break;
            case '}':
                read_and_advance();
                current_token = TOK_CURLY_CLOSED;
                break;
            case '[':
                read_and_advance();
                current_token = TOK_SQUARE_OPEN;
                break;
            case ']':
                read_and_advance();
                current_token = TOK_SQUARE_CLOSED;
                break;
            case '*':
                read_and_advance();
                current_token = TOK_BINOP_MUL;
                break;
            case '+':
                read_and_advance();
                current_token = TOK_BINOP_ADD;
                break;
            case '-':
                read_and_advance();
                current_token = TOK_BINOP_SUB;
                break;
            case '/':
                read_and_advance();
                current_token = TOK_BINOP_DIV;
                break;
            case '.':
                read_and_advance();
                current_token = TOK_DOT;
                break;
            case EOF:
                current_token = TOK_EOF;
                break;
            default:
                // By default, let us assume it is any token we dont know
                // about. Just to be safe.
                current_token = TOK_ANY;
                break;
        }
    }
    return current_token;
}
