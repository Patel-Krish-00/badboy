#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* ---- variable table ---- */
#define MAX_VARS 256
#define MAX_STRS 256

typedef struct { char name[64]; int offset; } VarEntry;

static VarEntry var_table[MAX_VARS];
static int num_vars;

/* ---- string literal table ---- */
static char str_values[MAX_STRS][256];
static int  num_strs;

/* ---- misc state ---- */
static int label_id;
static FILE *out;

/* ---- helpers ---- */
static int new_label() { return label_id++; }

static int find_var(const char *name) {
    for (int i = 0; i < num_vars; i++)
        if (strcmp(var_table[i].name, name) == 0)
            return var_table[i].offset;
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

static int find_or_add_str(const char *s) {
    for (int i = 0; i < num_strs; i++)
        if (strcmp(str_values[i], s) == 0) return i;
    strncpy(str_values[num_strs], s, 255);
    str_values[num_strs][255] = '\0';
    return num_strs++;
}

/* Escape a string for use in .asciz (GNU assembler) */
static void write_escaped(FILE *f, const char *s) {
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if      (c == '\n') fprintf(f, "\\n");
        else if (c == '\t') fprintf(f, "\\t");
        else if (c == '\\') fprintf(f, "\\\\");
        else if (c == '"')  fprintf(f, "\\\"");
        else                fputc(c, f);
    }
}

/* Pre-scan: collect all variable declarations (let / read) */
static void scan_vars(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_LET || node->type == AST_READ) {
        int found = 0;
        for (int i = 0; i < num_vars; i++)
            if (strcmp(var_table[i].name, node->value) == 0) { found = 1; break; }
        if (!found) {
            var_table[num_vars].offset = -(num_vars + 1) * 8;
            strncpy(var_table[num_vars].name, node->value, 63);
            var_table[num_vars].name[63] = '\0';
            num_vars++;
        }
    }
    scan_vars(node->left);
    scan_vars(node->right);
    scan_vars(node->extra);
}

/* Pre-scan: collect all string literals */
static void scan_strs(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_STRING) find_or_add_str(node->value);
    scan_strs(node->left);
    scan_strs(node->right);
    scan_strs(node->extra);
}

/* ---- code emission ---- */
static void emit_stmts(ASTNode *node);
static void emit_stmt(ASTNode *node);

/* Emit expression; result lands in rax */
static void emit_expr(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_INT:
            fprintf(out, "    mov rax, %d\n", node->int_val);
            break;
        case AST_STRING: {
            int id = find_or_add_str(node->value);
            fprintf(out, "    lea rax, [rip + .Lstr%d]\n", id);
            break;
        }
        case AST_VAR: {
            int off = find_var(node->value);
            fprintf(out, "    mov rax, [rbp%+d]\n", off);
            break;
        }
        case AST_BINOP: {
            const char *op = node->value;
            int is_cmp = (strcmp(op,"==")==0 || strcmp(op,"!=")==0 ||
                          strcmp(op,"<")==0  || strcmp(op,">")==0  ||
                          strcmp(op,"<=")==0 || strcmp(op,">=")==0);

            /* Evaluate left, push; evaluate right into rax; pop left into rbx */
            emit_expr(node->left);
            fprintf(out, "    push rax\n");
            emit_expr(node->right);
            fprintf(out, "    pop rbx\n");
            /* rbx = left operand, rax = right operand */

            if (is_cmp) {
                fprintf(out, "    cmp rbx, rax\n");
                if      (strcmp(op,"==")==0) fprintf(out, "    sete al\n");
                else if (strcmp(op,"!=")==0) fprintf(out, "    setne al\n");
                else if (strcmp(op,"<")==0)  fprintf(out, "    setl al\n");
                else if (strcmp(op,">")==0)  fprintf(out, "    setg al\n");
                else if (strcmp(op,"<=")==0) fprintf(out, "    setle al\n");
                else if (strcmp(op,">=")==0) fprintf(out, "    setge al\n");
                fprintf(out, "    movzx rax, al\n");
            } else if (strcmp(op,"+")==0) {
                fprintf(out, "    add rax, rbx\n");
            } else if (strcmp(op,"-")==0) {
                /* rbx - rax */
                fprintf(out, "    sub rbx, rax\n");
                fprintf(out, "    mov rax, rbx\n");
            } else if (strcmp(op,"*")==0) {
                fprintf(out, "    imul rax, rbx\n");
            } else if (strcmp(op,"/")==0) {
                /* rbx / rax */
                fprintf(out, "    mov rcx, rax\n");
                fprintf(out, "    mov rax, rbx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idiv rcx\n");
            } else if (strcmp(op,"%")==0) {
                /* rbx %% rax */
                fprintf(out, "    mov rcx, rax\n");
                fprintf(out, "    mov rax, rbx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idiv rcx\n");
                fprintf(out, "    mov rax, rdx\n");
            }
            break;
        }
        default: break;
    }
}

static void emit_stmt(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_WRITE: {
            ASTNode *expr = node->left;
            if (expr && expr->type == AST_STRING) {
                int id = find_or_add_str(expr->value);
                fprintf(out, "    lea rdi, [rip + .Lstr%d]\n", id);
                fprintf(out, "    call bad_print_str\n");
            } else {
                emit_expr(expr);
                fprintf(out, "    mov rdi, rax\n");
                fprintf(out, "    call bad_print_int\n");
            }
            break;
        }
        case AST_LET:
        case AST_ASSIGN: {
            emit_expr(node->left);
            int off = find_var(node->value);
            fprintf(out, "    mov [rbp%+d], rax\n", off);
            break;
        }
        case AST_READ: {
            fprintf(out, "    call bad_read_int\n");
            int off = find_var(node->value);
            fprintf(out, "    mov [rbp%+d], rax\n", off);
            break;
        }
        case AST_IF: {
            int lbl = new_label();
            emit_expr(node->left);
            fprintf(out, "    test rax, rax\n");
            if (node->extra) {
                fprintf(out, "    jz .Lelse%d\n", lbl);
                emit_stmts(node->right);
                fprintf(out, "    jmp .Lendif%d\n", lbl);
                fprintf(out, ".Lelse%d:\n", lbl);
                emit_stmts(node->extra);
                fprintf(out, ".Lendif%d:\n", lbl);
            } else {
                fprintf(out, "    jz .Lendif%d\n", lbl);
                emit_stmts(node->right);
                fprintf(out, ".Lendif%d:\n", lbl);
            }
            break;
        }
        case AST_WHILE: {
            int lbl = new_label();
            fprintf(out, ".Lwhile%d:\n", lbl);
            emit_expr(node->left);
            fprintf(out, "    test rax, rax\n");
            fprintf(out, "    jz .Lendwhile%d\n", lbl);
            emit_stmts(node->right);
            fprintf(out, "    jmp .Lwhile%d\n", lbl);
            fprintf(out, ".Lendwhile%d:\n", lbl);
            break;
        }
        default: break;
    }
}

static void emit_stmts(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_BLOCK || node->type == AST_PROGRAM) {
        ASTNode *curr = node;
        while (curr && curr->left) {
            emit_stmt(curr->left);
            curr = curr->right;
        }
    } else {
        emit_stmt(node);
    }
}

/* ---- runtime helpers written into the generated assembly ---- */
static void emit_runtime(void) {
    /* bad_print_int: print 64-bit integer in rdi followed by newline */
    fprintf(out,
        "bad_print_int:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    sub rsp, 32\n"
        "    mov rax, rdi\n"
        "    test rax, rax\n"
        "    jnz .Lpi_nonzero\n"
        "    mov byte ptr [rbp-2], 0x30\n"
        "    mov byte ptr [rbp-1], 0x0a\n"
        "    lea rsi, [rbp-2]\n"
        "    mov rax, 1\n"
        "    mov rdi, 1\n"
        "    mov rdx, 2\n"
        "    syscall\n"
        "    leave\n"
        "    ret\n"
        ".Lpi_nonzero:\n"
        "    xor r8d, r8d\n"
        "    test rax, rax\n"
        "    jns .Lpi_positive\n"
        "    neg rax\n"
        "    mov r8d, 1\n"
        ".Lpi_positive:\n"
        "    mov byte ptr [rbp-1], 0x0a\n"
        "    lea rcx, [rbp-2]\n"
        "    mov r9, 10\n"
        ".Lpi_loop:\n"
        "    xor rdx, rdx\n"
        "    div r9\n"
        "    add dl, 0x30\n"
        "    mov byte ptr [rcx], dl\n"
        "    dec rcx\n"
        "    test rax, rax\n"
        "    jnz .Lpi_loop\n"
        "    test r8d, r8d\n"
        "    jz .Lpi_no_minus\n"
        "    mov byte ptr [rcx], 0x2d\n"
        "    dec rcx\n"
        ".Lpi_no_minus:\n"
        "    inc rcx\n"
        "    lea rdx, [rbp-1]\n"
        "    sub rdx, rcx\n"
        "    inc rdx\n"
        "    mov rsi, rcx\n"
        "    mov rdi, 1\n"
        "    mov rax, 1\n"
        "    syscall\n"
        "    leave\n"
        "    ret\n\n"
    );

    /* bad_print_str: print null-terminated string in rdi followed by newline */
    fprintf(out,
        "bad_print_str:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    sub rsp, 16\n"
        "    mov rsi, rdi\n"
        "    xor rdx, rdx\n"
        ".Lps_len:\n"
        "    cmp byte ptr [rdi + rdx], 0\n"
        "    je .Lps_write\n"
        "    inc rdx\n"
        "    jmp .Lps_len\n"
        ".Lps_write:\n"
        "    mov rdi, 1\n"
        "    mov rax, 1\n"
        "    syscall\n"
        "    mov byte ptr [rbp-1], 0x0a\n"
        "    lea rsi, [rbp-1]\n"
        "    mov rdx, 1\n"
        "    mov rdi, 1\n"
        "    mov rax, 1\n"
        "    syscall\n"
        "    leave\n"
        "    ret\n\n"
    );

    /* bad_read_int: read integer from stdin, return value in rax */
    fprintf(out,
        "bad_read_int:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    sub rsp, 48\n"
        "    lea rsi, [rbp-40]\n"
        "    mov rax, 0\n"
        "    mov rdi, 0\n"
        "    mov rdx, 32\n"
        "    syscall\n"
        "    lea rsi, [rbp-40]\n"
        "    xor rax, rax\n"
        "    xor r8d, r8d\n"
        ".Lri_skip:\n"
        "    movzx rcx, byte ptr [rsi]\n"
        "    cmp rcx, 0x20\n"
        "    je .Lri_skip_adv\n"
        "    cmp rcx, 0x09\n"
        "    je .Lri_skip_adv\n"
        "    cmp rcx, 0x0a\n"
        "    je .Lri_skip_adv\n"
        "    cmp rcx, 0x0d\n"
        "    je .Lri_skip_adv\n"
        "    jmp .Lri_skip_done\n"
        ".Lri_skip_adv:\n"
        "    inc rsi\n"
        "    jmp .Lri_skip\n"
        ".Lri_skip_done:\n"
        "    cmp byte ptr [rsi], 0x2d\n"
        "    jne .Lri_parse\n"
        "    mov r8d, 1\n"
        "    inc rsi\n"
        ".Lri_parse:\n"
        "    movzx rcx, byte ptr [rsi]\n"
        "    cmp rcx, 0x30\n"
        "    jl .Lri_done\n"
        "    cmp rcx, 0x39\n"
        "    jg .Lri_done\n"
        "    sub rcx, 0x30\n"
        "    imul rax, rax, 10\n"
        "    add rax, rcx\n"
        "    inc rsi\n"
        "    jmp .Lri_parse\n"
        ".Lri_done:\n"
        "    test r8d, r8d\n"
        "    jz .Lri_ret\n"
        "    neg rax\n"
        ".Lri_ret:\n"
        "    leave\n"
        "    ret\n\n"
    );
}

/* ---- main entry point ---- */
void generate_asm(ASTNode *root, const char *filename) {
    /* reset state */
    num_vars = 0;
    num_strs = 0;
    label_id = 0;

    /* phase 1: collect variables and strings */
    scan_vars(root);
    scan_strs(root);

    /* compute stack frame size; (8 + stack_size) must be 0 mod 16
       so rsp is 16-byte aligned before any call instruction */
    int base = num_vars * 8;
    int stack_size = base + ((16 - (base + 8) % 16) % 16);

    out = fopen(filename, "w");
    if (!out) { perror("fopen"); exit(1); }

    /* Intel syntax */
    fprintf(out, ".intel_syntax noprefix\n\n");

    /* string literals in .rodata */
    if (num_strs > 0) {
        fprintf(out, ".section .rodata\n");
        for (int i = 0; i < num_strs; i++) {
            fprintf(out, ".Lstr%d:\n    .asciz \"", i);
            write_escaped(out, str_values[i]);
            fprintf(out, "\"\n");
        }
        fprintf(out, "\n");
    }

    /* code section */
    fprintf(out, ".section .text\n");
    fprintf(out, ".global _start\n\n");

    /* runtime helpers */
    emit_runtime();

    /* program entry */
    fprintf(out, "_start:\n");
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
    if (stack_size > 0)
        fprintf(out, "    sub rsp, %d\n", stack_size);

    /* emit program statements */
    emit_stmts(root);

    /* exit(0) */
    fprintf(out, "    mov rax, 60\n");
    fprintf(out, "    xor rdi, rdi\n");
    fprintf(out, "    syscall\n");

    fclose(out);
}