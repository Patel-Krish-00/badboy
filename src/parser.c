#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bad.h"
#include "ast.h"

static Token cur;

static void parser_advance() { cur = get_next_token(); }

static ASTNode* new_node(ASTNodeType type) {
    ASTNode* n = calloc(1, sizeof(ASTNode));
    if (!n) {
        fprintf(stderr, "Out of memory while creating AST node\n");
        exit(1);
    }
    n->type = type;
    return n;
}

static void eat(TokenType t) {
    if (cur.type == t) {
        parser_advance();
    } else {
        fprintf(stderr, "Syntax error: expected token type %d but got '%s' (type %d)\n",
                t, cur.value, cur.type);
        exit(1);
    }
}

/* Forward declarations */
static ASTNode* parse_expr();
static ASTNode* parse_stmt();
static ASTNode* parse_block();

/* Primary: int literal, string literal, variable, (expr), unary minus */
static ASTNode* parse_primary() {
    if (cur.type == TOKEN_INT) {
        ASTNode* n = new_node(AST_INT);
        n->int_val = atoi(cur.value);
        parser_advance();
        return n;
    }
    if (cur.type == TOKEN_STRING) {
        ASTNode* n = new_node(AST_STRING);
        strncpy(n->value, cur.value, 255);
        n->value[255] = '\0';
        parser_advance();
        return n;
    }
    if (cur.type == TOKEN_IDENT) {
        ASTNode* n = new_node(AST_VAR);
        strcpy(n->value, cur.value);
        parser_advance();
        return n;
    }
    if (cur.type == TOKEN_LPAREN) {
        eat(TOKEN_LPAREN);
        ASTNode* n = parse_expr();
        eat(TOKEN_RPAREN);
        return n;
    }
    if (cur.type == TOKEN_MINUS) {
        parser_advance();
        ASTNode* n = new_node(AST_BINOP);
        strcpy(n->value, "-");
        ASTNode* zero = new_node(AST_INT);
        zero->int_val = 0;
        n->left = zero;
        n->right = parse_primary();
        return n;
    }
    fprintf(stderr, "Parse error: unexpected token '%s'\n", cur.value);
    exit(1);
}

/* Multiplicative: *, /, % */
static ASTNode* parse_mul() {
    ASTNode* left = parse_primary();
    while (cur.type == TOKEN_STAR || cur.type == TOKEN_SLASH || cur.type == TOKEN_PERCENT) {
        ASTNode* n = new_node(AST_BINOP);
        strcpy(n->value, cur.value);
        parser_advance();
        n->left = left;
        n->right = parse_primary();
        left = n;
    }
    return left;
}

/* Additive: +, - */
static ASTNode* parse_add() {
    ASTNode* left = parse_mul();
    while (cur.type == TOKEN_PLUS || cur.type == TOKEN_MINUS) {
        ASTNode* n = new_node(AST_BINOP);
        strcpy(n->value, cur.value);
        parser_advance();
        n->left = left;
        n->right = parse_mul();
        left = n;
    }
    return left;
}

/* Comparison: ==, !=, <, >, <=, >= */
static ASTNode* parse_cmp() {
    ASTNode* left = parse_add();
    while (cur.type == TOKEN_EQ  || cur.type == TOKEN_NEQ ||
           cur.type == TOKEN_LT  || cur.type == TOKEN_GT  ||
           cur.type == TOKEN_LTE || cur.type == TOKEN_GTE) {
        ASTNode* n = new_node(AST_BINOP);
        strcpy(n->value, cur.value);
        parser_advance();
        n->left = left;
        n->right = parse_add();
        left = n;
    }
    return left;
}

static ASTNode* parse_expr() { return parse_cmp(); }

/* Parse a statement */
static ASTNode* parse_stmt() {
    /* write(expr); */
    if (cur.type == TOKEN_WRITE) {
        parser_advance();
        eat(TOKEN_LPAREN);
        ASTNode* n = new_node(AST_WRITE);
        n->left = parse_expr();
        eat(TOKEN_RPAREN);
        eat(TOKEN_SEMICOLON);
        return n;
    }
    /* let name = expr; */
    if (cur.type == TOKEN_LET) {
        parser_advance();
        ASTNode* n = new_node(AST_LET);
        strncpy(n->value, cur.value, 255);
        n->value[255] = '\0';
        eat(TOKEN_IDENT);
        eat(TOKEN_ASSIGN);
        n->left = parse_expr();
        eat(TOKEN_SEMICOLON);
        return n;
    }
    /* if (cond) block [else block] */
    if (cur.type == TOKEN_IF) {
        parser_advance();
        ASTNode* n = new_node(AST_IF);
        eat(TOKEN_LPAREN);
        n->left = parse_expr();
        eat(TOKEN_RPAREN);
        n->right = parse_block();
        if (cur.type == TOKEN_ELSE) {
            parser_advance();
            n->extra = parse_block();
        }
        return n;
    }
    /* while (cond) block */
    if (cur.type == TOKEN_WHILE) {
        parser_advance();
        ASTNode* n = new_node(AST_WHILE);
        eat(TOKEN_LPAREN);
        n->left = parse_expr();
        eat(TOKEN_RPAREN);
        n->right = parse_block();
        return n;
    }
    /* read(name); */
    if (cur.type == TOKEN_READ) {
        parser_advance();
        eat(TOKEN_LPAREN);
        ASTNode* n = new_node(AST_READ);
        strncpy(n->value, cur.value, 255);
        n->value[255] = '\0';
        eat(TOKEN_IDENT);
        eat(TOKEN_RPAREN);
        eat(TOKEN_SEMICOLON);
        return n;
    }
    /* name = expr; (assignment) */
    if (cur.type == TOKEN_IDENT) {
        char name[256];
        strcpy(name, cur.value);
        parser_advance();
        eat(TOKEN_ASSIGN);
        ASTNode* n = new_node(AST_ASSIGN);
        strcpy(n->value, name);
        n->left = parse_expr();
        eat(TOKEN_SEMICOLON);
        return n;
    }
    fprintf(stderr, "Parse error: unexpected token '%s'\n", cur.value);
    exit(1);
}

/* Block: { stmts } or a single statement */
static ASTNode* parse_block() {
    if (cur.type == TOKEN_LBRACE) {
        eat(TOKEN_LBRACE);
        ASTNode* root = new_node(AST_BLOCK);
        ASTNode* curr = root;
        while (cur.type != TOKEN_RBRACE && cur.type != TOKEN_EOF) {
            curr->left = parse_stmt();
            if (cur.type != TOKEN_RBRACE && cur.type != TOKEN_EOF) {
                curr->right = new_node(AST_BLOCK);
                curr = curr->right;
            }
        }
        eat(TOKEN_RBRACE);
        return root;
    }
    return parse_stmt();
}

/* Top-level program */
ASTNode* parse() {
    parser_advance();
    ASTNode* root = new_node(AST_PROGRAM);
    ASTNode* curr = root;
    while (cur.type != TOKEN_EOF) {
        curr->left = parse_stmt();
        if (cur.type != TOKEN_EOF) {
            curr->right = new_node(AST_PROGRAM);
            curr = curr->right;
        }
    }
    return root;
}