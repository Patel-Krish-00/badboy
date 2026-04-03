#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bad.h"

const char *input;
int pos = 0;

// Initialize lexer
void init_lexer(const char *src) {
    input = src;
    pos = 0;
}

void advance() {
    pos++;
}

// Get current character
char current_char() {
    return input[pos];
}

// Skip spaces
void skip_whitespace() {
    while (isspace(current_char())) {
        advance();
    }
}

// Read string: 'hello'
Token read_string() {
    Token token;
    token.type = TOKEN_STRING;

    advance(); // skip '
    int i = 0;
    while (current_char() != '\'' && current_char() != '\0') {
        token.value[i++] = current_char();
        advance();
    }

    token.value[i] = '\0';

    if (current_char() == '\'') {
        advance(); // skip closing '
    }

    return token;
}

// Read identifier
Token read_identifier() {
    Token token;
    char buffer[256];
    int i = 0;

    while (isalnum(current_char())) {
        buffer[i++] = current_char();
        advance();
    }

    buffer[i] = '\0';

    if (strcmp(buffer, "write") == 0) {
        token.type = TOKEN_WRITE;
    } else {
        token.type = TOKEN_UNKNOWN;
    }

    strcpy(token.value, buffer);
    return token;
}

// Main lexer function
Token get_next_token() {
    skip_whitespace();

    char c = current_char();

    if (c == '\0') {
        return (Token){TOKEN_EOF, ""};
    }

    if (isalpha(c)) {
        return read_identifier();
    }

    if (c == '\'') {
        return read_string();
    }

    if (c == '(') {
        advance();
        return (Token){TOKEN_LPAREN, "("};
    }

    if (c == ')') {
        advance();
        return (Token){TOKEN_RPAREN, ")"};
    }

    if (c == ';') {
        advance();
        return (Token){TOKEN_SEMICOLON, ";"};
    }

    // Unknown character
    advance();
    return (Token){TOKEN_UNKNOWN, ""};
}