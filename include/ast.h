#ifndef AST_H
#define AST_H

typedef enum {
    AST_PROGRAM,   /* sequence: left=stmt, right=next */
    AST_WRITE,     /* write(expr): left=expr */
    AST_LET,       /* let name=expr: value=name, left=expr */
    AST_ASSIGN,    /* name=expr: value=name, left=expr */
    AST_IF,        /* if(cond) body [else body]: left=cond, right=then, extra=else */
    AST_WHILE,     /* while(cond) body: left=cond, right=body */
    AST_BLOCK,     /* { stmts }: left=stmt, right=next */
    AST_READ,      /* read(name): value=name */
    AST_VAR,       /* variable reference: value=name */
    AST_INT,       /* integer literal: int_val */
    AST_STRING,    /* string literal: value=content */
    AST_BINOP,     /* binary op: value=op, left=left, right=right */
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char value[256];   /* variable name, string content, or operator */
    int int_val;       /* integer literal value */
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* extra; /* else body for AST_IF */
} ASTNode;

#endif