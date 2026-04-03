#include <stdio.h>
#include "ast.h"

// Execute AST
void execute(ASTNode* node) {
    if (node->type == AST_WRITE) {
        printf("%s\n", node->value);
    }
}