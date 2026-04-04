#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bad.h"

static const char *input;
static int pos;

void init_lexer(const char *src) {
    input = src;
    pos = 0;
}

static void advance() { pos++; }
static char current_char() { return input[pos]; }
static char peek_char()    { return input[pos + 1]; }

static void skip_whitespace_and_comments() {
    for (;;) {
        while (isspace((unsigned char)current_char())) advance();
        if (current_char() == '/' && peek_char() == '/') {
            while (current_char() != '\n' && current_char() != '\0') advance();
        } else {
            break;
        }
    }
}

static Token read_string() {
    Token tok;
    tok.type = TOKEN_STRING;
    advance(); /* skip opening ' */
    int i = 0;
    while (current_char() != '\'' && current_char() != '\0') {
        if (current_char() == '\\') {
            advance();
            switch (current_char()) {
                case 'n':  tok.value[i++] = '\n'; break;
                case 't':  tok.value[i++] = '\t'; break;
                case '\'': tok.value[i++] = '\''; break;
                case '\\': tok.value[i++] = '\\'; break;
                default:   tok.value[i++] = current_char(); break;
            }
        } else {
            tok.value[i++] = current_char();
        }
        advance();
    }
    tok.value[i] = '\0';
    if (current_char() == '\'') advance(); /* skip closing ' */
    return tok;
}

static Token read_number() {
    Token tok;
    tok.type = TOKEN_INT;
    int i = 0;
    while (isdigit((unsigned char)current_char())) {
        tok.value[i++] = current_char();
        advance();
    }
    tok.value[i] = '\0';
    return tok;
}

static Token read_identifier() {
    Token tok;
    char buf[256];
    int i = 0;
    while (isalnum((unsigned char)current_char()) || current_char() == '_') {
        buf[i++] = current_char();
        advance();
    }
    buf[i] = '\0';
    strcpy(tok.value, buf);

    if      (strcmp(buf, "write") == 0) tok.type = TOKEN_WRITE;
    else if (strcmp(buf, "let")   == 0) tok.type = TOKEN_LET;
    else if (strcmp(buf, "if")    == 0) tok.type = TOKEN_IF;
    else if (strcmp(buf, "else")  == 0) tok.type = TOKEN_ELSE;
    else if (strcmp(buf, "while") == 0) tok.type = TOKEN_WHILE;
    else if (strcmp(buf, "read")  == 0) tok.type = TOKEN_READ;
    else                                tok.type = TOKEN_IDENT;

    return tok;
}

Token get_next_token() {
    skip_whitespace_and_comments();

    char c = current_char();
    if (c == '\0') return (Token){TOKEN_EOF, ""};

    if (isalpha((unsigned char)c) || c == '_') return read_identifier();
    if (isdigit((unsigned char)c))             return read_number();
    if (c == '\'')                             return read_string();

    Token tok;
    tok.value[0] = c;
    tok.value[1] = '\0';

    switch (c) {
        case '(': advance(); tok.type = TOKEN_LPAREN;    return tok;
        case ')': advance(); tok.type = TOKEN_RPAREN;    return tok;
        case '{': advance(); tok.type = TOKEN_LBRACE;    return tok;
        case '}': advance(); tok.type = TOKEN_RBRACE;    return tok;
        case ';': advance(); tok.type = TOKEN_SEMICOLON; return tok;
        case ',': advance(); tok.type = TOKEN_COMMA;     return tok;
        case '+': advance(); tok.type = TOKEN_PLUS;      return tok;
        case '-': advance(); tok.type = TOKEN_MINUS;     return tok;
        case '*': advance(); tok.type = TOKEN_STAR;      return tok;
        case '/': advance(); tok.type = TOKEN_SLASH;     return tok;
        case '%': advance(); tok.type = TOKEN_PERCENT;   return tok;
        case '=':
            advance();
            if (current_char() == '=') {
                advance();
                strcpy(tok.value, "==");
                tok.type = TOKEN_EQ;
            } else {
                tok.type = TOKEN_ASSIGN;
            }
            return tok;
        case '!':
            advance();
            if (current_char() == '=') {
                advance();
                strcpy(tok.value, "!=");
                tok.type = TOKEN_NEQ;
            } else {
                tok.type = TOKEN_UNKNOWN;
            }
            return tok;
        case '<':
            advance();
            if (current_char() == '=') {
                advance();
                strcpy(tok.value, "<=");
                tok.type = TOKEN_LTE;
            } else {
                tok.type = TOKEN_LT;
            }
            return tok;
        case '>':
            advance();
            if (current_char() == '=') {
                advance();
                strcpy(tok.value, ">=");
                tok.type = TOKEN_GTE;
            } else {
                tok.type = TOKEN_GT;
            }
            return tok;
        default:
            advance();
            tok.type = TOKEN_UNKNOWN;
            return tok;
    }
}