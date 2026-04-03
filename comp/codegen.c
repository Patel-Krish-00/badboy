#include <stdio.h>
#include "ast.h"

// forward
void generate_node(ASTNode* node, FILE* out);

// traverse program
void generate_program(ASTNode* node, FILE* out) {
    while (node != NULL && node->left != NULL) {
        generate_node(node->left, out);
        node = node->right;
    }
}

void generate_node(ASTNode* node, FILE* out) {
    if (node->type == AST_WRITE) {
        fprintf(out, "printf(\"%s\\n\");\n", node->value);
    }
}

// entry
void generate_c(ASTNode* root, const char* filename) {
    FILE* out = fopen(filename, "w");

    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "int main() {\n");

    generate_program(root, out);

    fprintf(out, "return 0;\n}\n");

    fclose(out);
}