#ifndef BAD_H
#define BAD_H

typedef enum {
    TOKEN_WRITE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_STRING,
    TOKEN_SEMICOLON,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
} Token;

// lexer functions
void init_lexer(const char *src);
Token get_next_token();

// parser
struct ASTNode;
typedef struct ASTNode ASTNode;

ASTNode* parse();

// executor
struct ASTNode;
void execute(struct ASTNode* node);

// codegen
struct ASTNode;
void generate_c(struct ASTNode* root, const char* filename);

void generate_asm(struct ASTNode* root, const char* filename);

#endif