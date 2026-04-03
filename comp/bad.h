#ifndef BAD_H
#define BAD_H

typedef enum {
    /* keywords */
    TOKEN_WRITE,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_READ,
    /* literals */
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_IDENT,
    /* punctuation */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    /* operators */
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    /* comparison */
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTE,
    TOKEN_GTE,
    /* special */
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
} Token;

void init_lexer(const char *src);
Token get_next_token();

struct ASTNode;
typedef struct ASTNode ASTNode;

ASTNode* parse();
void execute(ASTNode* node);
void generate_c(ASTNode* root, const char* filename);
void generate_asm(ASTNode* root, const char* filename);

#endif