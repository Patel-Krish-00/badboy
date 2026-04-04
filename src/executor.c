#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_VARS 256

typedef struct { char name[64]; long long value; } ExecVar;
static ExecVar exec_vars[MAX_VARS];
static int exec_num_vars;

static long long get_var(const char *name) {
    for (int i = 0; i < exec_num_vars; i++)
        if (strcmp(exec_vars[i].name, name) == 0) return exec_vars[i].value;
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

static void set_var(const char *name, long long val) {
    for (int i = 0; i < exec_num_vars; i++) {
        if (strcmp(exec_vars[i].name, name) == 0) { exec_vars[i].value = val; return; }
    }
    strncpy(exec_vars[exec_num_vars].name, name, 63);
    exec_vars[exec_num_vars].name[63] = '\0';
    exec_vars[exec_num_vars].value = val;
    exec_num_vars++;
}

static long long eval_expr(ASTNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case AST_INT:    return (long long)node->int_val;
        case AST_STRING: return 0;
        case AST_VAR:    return get_var(node->value);
        case AST_BINOP: {
            long long l = eval_expr(node->left);
            long long r = eval_expr(node->right);
            const char *op = node->value;
            if      (strcmp(op,"+")==0)  return l + r;
            else if (strcmp(op,"-")==0)  return l - r;
            else if (strcmp(op,"*")==0)  return l * r;
            else if (strcmp(op,"/")==0)  return r ? l / r : 0;
            else if (strcmp(op,"%")==0)  return r ? l % r : 0;
            else if (strcmp(op,"==")==0) return l == r;
            else if (strcmp(op,"!=")==0) return l != r;
            else if (strcmp(op,"<")==0)  return l < r;
            else if (strcmp(op,">")==0)  return l > r;
            else if (strcmp(op,"<=")==0) return l <= r;
            else if (strcmp(op,">=")==0) return l >= r;
            return 0;
        }
        default: return 0;
    }
}

static void exec_stmts(ASTNode *node);

static void exec_stmt(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_WRITE: {
            ASTNode *e = node->left;
            if (e && e->type == AST_STRING) printf("%s\n", e->value);
            else printf("%lld\n", (long long)eval_expr(e));
            break;
        }
        case AST_LET:
        case AST_ASSIGN:
            set_var(node->value, eval_expr(node->left));
            break;
        case AST_READ: {
            long long v = 0;
            scanf("%lld", &v);
            set_var(node->value, v);
            break;
        }
        case AST_IF:
            if (eval_expr(node->left)) exec_stmts(node->right);
            else if (node->extra)      exec_stmts(node->extra);
            break;
        case AST_WHILE:
            while (eval_expr(node->left)) exec_stmts(node->right);
            break;
        default: break;
    }
}

static void exec_stmts(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_BLOCK || node->type == AST_PROGRAM) {
        ASTNode *curr = node;
        while (curr && curr->left) { exec_stmt(curr->left); curr = curr->right; }
    } else {
        exec_stmt(node);
    }
}

void execute(ASTNode *node) {
    exec_num_vars = 0;
    exec_stmts(node);
}