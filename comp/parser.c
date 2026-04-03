#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bad.h"
#include "ast.h"

Token current_token;

void parser_advance() {
    current_token = get_next_token();
}

void eat(TokenType type) {
    if (current_token.type == type) {
        parser_advance();
    } else {
        printf("Syntax Error: Unexpected token %s\n", current_token.value);
        exit(1);
    }
}

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// parse write('...')
ASTNode* parse_write() {
    ASTNode* node = create_node(AST_WRITE);

    eat(TOKEN_WRITE);
    eat(TOKEN_LPAREN);

    if (current_token.type != TOKEN_STRING) {
        printf("Expected string\n");
        exit(1);
    }

    strcpy(node->value, current_token.value);
    eat(TOKEN_STRING);

    eat(TOKEN_RPAREN);
    eat(TOKEN_SEMICOLON);

    return node;
}

// parse multiple statements
ASTNode* parse_program() {
    ASTNode* root = create_node(AST_PROGRAM);
    ASTNode* current = root;

    while (current_token.type != TOKEN_EOF) {
        current->left = parse_write();

        if (current_token.type != TOKEN_EOF) {
            current->right = create_node(AST_PROGRAM);
            current = current->right;
        }
    }

    return root;
}

// entry point
ASTNode* parse() {
    parser_advance();
    return parse_program();
}