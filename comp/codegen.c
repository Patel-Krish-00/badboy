#include <stdio.h>
#include <string.h>
#include "ast.h"

static void gen_node(ASTNode *node, FILE *out);

static void gen_expr(ASTNode *node, FILE *out) {
    if (!node) return;
    switch (node->type) {
        case AST_INT:
            fprintf(out, "%d", node->int_val);
            break;
        case AST_STRING:
            fprintf(out, "\"%s\"", node->value);
            break;
        case AST_VAR:
            fprintf(out, "%s", node->value);
            break;
        case AST_BINOP:
            fprintf(out, "(");
            gen_expr(node->left, out);
            fprintf(out, " %s ", node->value);
            gen_expr(node->right, out);
            fprintf(out, ")");
            break;
        default: break;
    }
}

static void gen_stmts(ASTNode *node, FILE *out);

static void gen_node(ASTNode *node, FILE *out) {
    if (!node) return;
    switch (node->type) {
        case AST_WRITE: {
            ASTNode *e = node->left;
            if (e && e->type == AST_STRING) {
                fprintf(out, "    printf(\"%%s\\n\", ");
                gen_expr(e, out);
                fprintf(out, ");\n");
            } else {
                fprintf(out, "    printf(\"%%lld\\n\", (long long)(");
                gen_expr(e, out);
                fprintf(out, "));\n");
            }
            break;
        }
        case AST_LET:
            fprintf(out, "    long long %s = ", node->value);
            gen_expr(node->left, out);
            fprintf(out, ";\n");
            break;
        case AST_ASSIGN:
            fprintf(out, "    %s = ", node->value);
            gen_expr(node->left, out);
            fprintf(out, ";\n");
            break;
        case AST_READ:
            fprintf(out, "    scanf(\"%%lld\", &%s);\n", node->value);
            break;
        case AST_IF:
            fprintf(out, "    if (");
            gen_expr(node->left, out);
            fprintf(out, ") {\n");
            gen_stmts(node->right, out);
            fprintf(out, "    }");
            if (node->extra) {
                fprintf(out, " else {\n");
                gen_stmts(node->extra, out);
                fprintf(out, "    }");
            }
            fprintf(out, "\n");
            break;
        case AST_WHILE:
            fprintf(out, "    while (");
            gen_expr(node->left, out);
            fprintf(out, ") {\n");
            gen_stmts(node->right, out);
            fprintf(out, "    }\n");
            break;
        default: break;
    }
}

static void gen_stmts(ASTNode *node, FILE *out) {
    if (!node) return;
    if (node->type == AST_BLOCK || node->type == AST_PROGRAM) {
        ASTNode *curr = node;
        while (curr && curr->left) {
            gen_node(curr->left, out);
            curr = curr->right;
        }
    } else {
        gen_node(node, out);
    }
}

void generate_c(ASTNode *root, const char *filename) {
    FILE *out = fopen(filename, "w");
    if (!out) { perror("fopen"); return; }

    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "int main(void) {\n");
    gen_stmts(root, out);
    fprintf(out, "    return 0;\n}\n");

    fclose(out);
}