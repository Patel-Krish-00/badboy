#ifndef AST_H
#define AST_H

typedef enum {
    AST_WRITE,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    char value[256];   // for write

    struct ASTNode* left;   // for list chaining
    struct ASTNode* right;

} ASTNode;

#endif