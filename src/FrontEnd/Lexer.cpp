#include "Lexer.h"

using namespace papyrus;

#define RESERVE_WORD(a, b) reserved_words_.insert({a,b})

char Lexer::ReadChar() {
    last_char_ = ch_;
    input_buffer_.get(ch_); 

    if (input_buffer_.eof()) return EOF;
    return ch_;
}

char Lexer::PeekChar() {
    if (input_buffer_.eof()) return EOF;

    return input_buffer_.peek();
}

void Lexer::ReadAndAdvance() {
    current_buffer_.push_back(ReadChar());
}

void Lexer::ConsumeLine() {
    char peek_char = PeekChar();
    while (peek_char != EOF && peek_char != '\n') {
        ReadChar();
        peek_char= PeekChar();
    }
}

void Lexer::ConsumeWhitespaceAndComments() {
    char peek_char = PeekChar();
    while (peek_char != EOF) {
        if (peek_char == ' ' || peek_char == '\t' || peek_char == '\r') {
            ReadChar();
        } else if (peek_char == '\n') {
            current_lineno_ += 1;
            ReadChar();
        } else if (peek_char == '#') {
            ConsumeLine();
        } else if (PeekChar() == '/') {
            ReadChar();
            if (PeekChar() == '/')
                ConsumeLine();
            else {
                LOG(ERROR) << "[LEXER] Invalid character found after '/' on Line: " << current_lineno_;
            }
        } else
            break;

        peek_char = PeekChar();
    }
}

Lexer::Lexer(std::istream &i_buf) : 
    input_buffer_(i_buf),
    current_buffer_(),
    current_lineno_(1),
    ch_('\0'),
    last_char_(' '),
    current_token_(Lexer::TOK_NONE),
    reserved_words_(),
    LOGCFG_() {
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
    

void Lexer::Reserve(std::string& word, Lexer::Token& token) {
    RESERVE_WORD(word, token);
}

Lexer::Token Lexer::GetNextToken() {
    current_buffer_ = "";
    current_token_ = Token::TOK_NONE;

    ConsumeWhitespaceAndComments();
        
    if (std::isalpha(PeekChar())) {
        ReadAndAdvance();
        while (std::isalnum(PeekChar())) {
            ReadAndAdvance();
        }
        auto res_word_it = reserved_words_.find(current_buffer_);
        if (res_word_it != reserved_words_.end()) {
            // Found a Reserved word!
            // Return token since we have already stored it.
            current_token_ = res_word_it->second;
        } else {
            current_token_ = TOK_IDENT;
        }
    } else if (std::isdigit(PeekChar())) {
        ReadAndAdvance();
        while (std::isdigit(PeekChar())) {
            ReadAndAdvance();
        }
        if (std::isalpha(PeekChar())) {
            LOG(ERROR) << "[LEXER] Invalid number literal found on Line: " << current_lineno_;
        } else {
            current_token_ = TOK_NUM;
        }
    } else {
        switch(PeekChar()) {
            case '!':
            case '=':
                ReadAndAdvance();
                if (PeekChar() == '=') {
                    ReadAndAdvance();
                    current_token_ = last_char_ == '!' ? TOK_RELOP_NEQ : TOK_RELOP_EQ;
                } else {
                    LOG(ERROR) << "[LEXER] Invalid character found after " << last_char_ << " on Line: " << current_lineno_;
                }
                break;
            case '<':
                ReadAndAdvance();
                if (PeekChar() == '-') {
                    ReadAndAdvance();
                    current_token_ = TOK_LEFTARROW;
                } else {
                    if (PeekChar() == '=') {
                        ReadAndAdvance();
                        current_token_ = TOK_RELOP_LTE;
                    } else {
                        ReadAndAdvance();
                        current_token_ = TOK_RELOP_LT;
                    }
                }
                break;
            case '>':
                ReadAndAdvance();
                if (PeekChar() == '=') {
                    ReadAndAdvance();
                    current_token_ = TOK_RELOP_GTE;
                } else {
                    ReadAndAdvance();
                    current_token_ = TOK_RELOP_GT;
                }
                break;
            case ';':
                ReadAndAdvance();
                current_token_ = TOK_SEMICOLON;
                break;
            case ',':
                ReadAndAdvance();
                current_token_ = TOK_COMMA;
                break;
            case '(':
                ReadAndAdvance();
                current_token_ = TOK_ROUND_OPEN;
                break;
            case ')':
                ReadAndAdvance();
                current_token_ = TOK_ROUND_CLOSED;
                break;
            case '{':
                ReadAndAdvance();
                current_token_ = TOK_CURLY_OPEN;
                break;
            case '}':
                ReadAndAdvance();
                current_token_ = TOK_CURLY_CLOSED;
                break;
            case '[':
                ReadAndAdvance();
                current_token_ = TOK_SQUARE_OPEN;
                break;
            case ']':
                ReadAndAdvance();
                current_token_ = TOK_SQUARE_CLOSED;
                break;
            case '*':
                ReadAndAdvance();
                current_token_ = TOK_BINOP_MUL;
                break;
            case '+':
                ReadAndAdvance();
                current_token_ = TOK_BINOP_ADD;
                break;
            case '-':
                ReadAndAdvance();
                current_token_ = TOK_BINOP_SUB;
                break;
            case '/':
                ReadAndAdvance();
                current_token_ = TOK_BINOP_DIV;
                break;
            case '.':
                ReadAndAdvance();
                current_token_ = TOK_DOT;
                break;
            case EOF:
                current_token_ = TOK_EOF;
                break;
            default:
                // By default, let us assume it is any token we dont know
                // about. Just to be safe.
                current_token_ = TOK_ANY;
                break;
        }
    }
    return current_token_;
}
