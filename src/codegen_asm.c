#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_VARS 256
#define MAX_STRS 256

typedef struct { char name[64]; int offset; } VarEntry;

static VarEntry var_table[MAX_VARS];
static int num_vars;

static char str_values[MAX_STRS][256];
static int num_strs;

static int label_id;
static FILE *out;

#if defined(__APPLE__)
#define ASM_MAIN_LABEL "_main"
#define ASM_PUTS "_puts"
#define ASM_BAD_WRITE_INT "_bad_write_int"
#define ASM_BAD_READ_INT "_bad_read_int"
#else
#define ASM_MAIN_LABEL "main"
#define ASM_PUTS "puts"
#define ASM_BAD_WRITE_INT "bad_write_int"
#define ASM_BAD_READ_INT "bad_read_int"
#endif

#if defined(_WIN32)
#define X86_ARG1 "rcx"
#define X86_ARG2 "rdx"
#else
#define X86_ARG1 "rdi"
#define X86_ARG2 "rsi"
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define TARGET_ARM64 1
#else
#define TARGET_ARM64 0
#endif

static int new_label(void) { return label_id++; }

static int find_var(const char *name) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(var_table[i].name, name) == 0) return var_table[i].offset;
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

static int find_or_add_str(const char *s) {
    for (int i = 0; i < num_strs; i++) {
        if (strcmp(str_values[i], s) == 0) return i;
    }
    if (num_strs >= MAX_STRS) {
        fprintf(stderr, "Too many string literals\n");
        exit(1);
    }
    strncpy(str_values[num_strs], s, 255);
    str_values[num_strs][255] = '\0';
    return num_strs++;
}

static void write_escaped(FILE *f, const char *s) {
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '\n') fprintf(f, "\\n");
        else if (c == '\t') fprintf(f, "\\t");
        else if (c == '\\') fprintf(f, "\\\\");
        else if (c == '"') fprintf(f, "\\\"");
        else fputc(c, f);
    }
}

static void scan_vars(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_LET || node->type == AST_READ) {
        int found = 0;
        for (int i = 0; i < num_vars; i++) {
            if (strcmp(var_table[i].name, node->value) == 0) { found = 1; break; }
        }
        if (!found) {
            if (num_vars >= MAX_VARS) {
                fprintf(stderr, "Too many variables\n");
                exit(1);
            }
            var_table[num_vars].offset = (num_vars + 1) * 8;
            strncpy(var_table[num_vars].name, node->value, 63);
            var_table[num_vars].name[63] = '\0';
            num_vars++;
        }
    }
    scan_vars(node->left);
    scan_vars(node->right);
    scan_vars(node->extra);
}

static void scan_strs(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_STRING) find_or_add_str(node->value);
    scan_strs(node->left);
    scan_strs(node->right);
    scan_strs(node->extra);
}

static void emit_stmts(ASTNode *node);
static void emit_stmt(ASTNode *node);
static void emit_expr(ASTNode *node);

#if TARGET_ARM64

static void emit_call(const char *symbol) {
    fprintf(out, "    bl %s\n", symbol);
}

static void emit_load_addr(const char *reg, const char *label) {
#if defined(__APPLE__)
    fprintf(out, "    adrp %s, %s@PAGE\n", reg, label);
    fprintf(out, "    add %s, %s, %s@PAGEOFF\n", reg, reg, label);
#else
    fprintf(out, "    adrp %s, %s\n", reg, label);
    fprintf(out, "    add %s, %s, :lo12:%s\n", reg, reg, label);
#endif
}

static void emit_load_var_addr(const char *reg, int off) {
    fprintf(out, "    sub %s, x29, #%d\n", reg, off);
}

static void emit_expr(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_INT:
            fprintf(out, "    mov x0, %d\n", node->int_val);
            break;
        case AST_STRING: {
            int id = find_or_add_str(node->value);
            char label[32];
            snprintf(label, sizeof(label), ".Lstr%d", id);
            emit_load_addr("x0", label);
            break;
        }
        case AST_VAR: {
            int off = find_var(node->value);
            emit_load_var_addr("x9", off);
            fprintf(out, "    ldr x0, [x9]\n");
            break;
        }
        case AST_BINOP: {
            const char *op = node->value;
            int is_cmp = (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                          strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
                          strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0);

            emit_expr(node->left);
            fprintf(out, "    str x0, [sp, -16]!\n");
            emit_expr(node->right);
            fprintf(out, "    ldr x9, [sp], 16\n");

            if (is_cmp) {
                fprintf(out, "    cmp x9, x0\n");
                if (strcmp(op, "==") == 0) fprintf(out, "    cset x0, eq\n");
                else if (strcmp(op, "!=") == 0) fprintf(out, "    cset x0, ne\n");
                else if (strcmp(op, "<") == 0) fprintf(out, "    cset x0, lt\n");
                else if (strcmp(op, ">") == 0) fprintf(out, "    cset x0, gt\n");
                else if (strcmp(op, "<=") == 0) fprintf(out, "    cset x0, le\n");
                else if (strcmp(op, ">=") == 0) fprintf(out, "    cset x0, ge\n");
            } else if (strcmp(op, "+") == 0) {
                fprintf(out, "    add x0, x0, x9\n");
            } else if (strcmp(op, "-") == 0) {
                fprintf(out, "    sub x0, x9, x0\n");
            } else if (strcmp(op, "*") == 0) {
                fprintf(out, "    mul x0, x0, x9\n");
            } else if (strcmp(op, "/") == 0) {
                fprintf(out, "    sdiv x0, x9, x0\n");
            } else if (strcmp(op, "%") == 0) {
                fprintf(out, "    sdiv x10, x9, x0\n");
                fprintf(out, "    msub x0, x10, x0, x9\n");
            }
            break;
        }
        default:
            break;
    }
}

static void emit_stmt(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_WRITE: {
            ASTNode *expr = node->left;
            if (expr && expr->type == AST_STRING) {
                int id = find_or_add_str(expr->value);
                char label[32];
                snprintf(label, sizeof(label), ".Lstr%d", id);
                emit_load_addr("x0", label);
                emit_call(ASM_PUTS);
            } else {
                emit_expr(expr);
                emit_call(ASM_BAD_WRITE_INT);
            }
            break;
        }
        case AST_LET:
        case AST_ASSIGN: {
            emit_expr(node->left);
            int off = find_var(node->value);
            emit_load_var_addr("x9", off);
            fprintf(out, "    str x0, [x9]\n");
            break;
        }
        case AST_READ: {
            int off = find_var(node->value);
            emit_load_var_addr("x0", off);
            emit_call(ASM_BAD_READ_INT);
            break;
        }
        case AST_IF: {
            int lbl = new_label();
            emit_expr(node->left);
            fprintf(out, "    cbz x0, .Lelse%d\n", lbl);
            emit_stmts(node->right);
            if (node->extra) {
                fprintf(out, "    b .Lendif%d\n", lbl);
                fprintf(out, ".Lelse%d:\n", lbl);
                emit_stmts(node->extra);
                fprintf(out, ".Lendif%d:\n", lbl);
            } else {
                fprintf(out, ".Lelse%d:\n", lbl);
            }
            break;
        }
        case AST_WHILE: {
            int lbl = new_label();
            fprintf(out, ".Lwhile%d:\n", lbl);
            emit_expr(node->left);
            fprintf(out, "    cbz x0, .Lendwhile%d\n", lbl);
            emit_stmts(node->right);
            fprintf(out, "    b .Lwhile%d\n", lbl);
            fprintf(out, ".Lendwhile%d:\n", lbl);
            break;
        }
        default:
            break;
    }
}

#else

static void emit_call(const char *symbol) {
#if defined(_WIN32)
    fprintf(out, "    sub rsp, 32\n");
    fprintf(out, "    call %s\n", symbol);
    fprintf(out, "    add rsp, 32\n");
#else
    fprintf(out, "    call %s\n", symbol);
#endif
}

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
            fprintf(out, "    mov rax, [rbp-%d]\n", off);
            break;
        }
        case AST_BINOP: {
            const char *op = node->value;
            int is_cmp = (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                          strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
                          strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0);

            emit_expr(node->left);
            fprintf(out, "    push rax\n");
            emit_expr(node->right);
            fprintf(out, "    pop rcx\n");

            if (is_cmp) {
                fprintf(out, "    cmp rcx, rax\n");
                if (strcmp(op, "==") == 0) fprintf(out, "    sete al\n");
                else if (strcmp(op, "!=") == 0) fprintf(out, "    setne al\n");
                else if (strcmp(op, "<") == 0) fprintf(out, "    setl al\n");
                else if (strcmp(op, ">") == 0) fprintf(out, "    setg al\n");
                else if (strcmp(op, "<=") == 0) fprintf(out, "    setle al\n");
                else if (strcmp(op, ">=") == 0) fprintf(out, "    setge al\n");
                fprintf(out, "    movzx rax, al\n");
            } else if (strcmp(op, "+") == 0) {
                fprintf(out, "    add rax, rcx\n");
            } else if (strcmp(op, "-") == 0) {
                fprintf(out, "    sub rcx, rax\n");
                fprintf(out, "    mov rax, rcx\n");
            } else if (strcmp(op, "*") == 0) {
                fprintf(out, "    imul rax, rcx\n");
            } else if (strcmp(op, "/") == 0) {
                fprintf(out, "    mov r10, rax\n");
                fprintf(out, "    mov rax, rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idiv r10\n");
            } else if (strcmp(op, "%") == 0) {
                fprintf(out, "    mov r10, rax\n");
                fprintf(out, "    mov rax, rcx\n");
                fprintf(out, "    cqo\n");
                fprintf(out, "    idiv r10\n");
                fprintf(out, "    mov rax, rdx\n");
            }
            break;
        }
        default:
            break;
    }
}

static void emit_stmt(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_WRITE: {
            ASTNode *expr = node->left;
            if (expr && expr->type == AST_STRING) {
                int id = find_or_add_str(expr->value);
                fprintf(out, "    lea %s, [rip + .Lstr%d]\n", X86_ARG1, id);
                emit_call(ASM_PUTS);
            } else {
                emit_expr(expr);
                fprintf(out, "    mov %s, rax\n", X86_ARG1);
                emit_call(ASM_BAD_WRITE_INT);
            }
            break;
        }
        case AST_LET:
        case AST_ASSIGN: {
            emit_expr(node->left);
            int off = find_var(node->value);
            fprintf(out, "    mov [rbp-%d], rax\n", off);
            break;
        }
        case AST_READ: {
            int off = find_var(node->value);
            fprintf(out, "    lea %s, [rbp-%d]\n", X86_ARG1, off);
            emit_call(ASM_BAD_READ_INT);
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
        default:
            break;
    }
}

#endif

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

void generate_asm(ASTNode *root, const char *filename) {
    num_vars = 0;
    num_strs = 0;
    label_id = 0;

    scan_vars(root);
    scan_strs(root);

    int base = num_vars * 8;
    int stack_size = (base + 15) & ~15;

    out = fopen(filename, "w");
    if (!out) { perror("fopen"); exit(1); }

#if TARGET_ARM64
    fprintf(out, "\n");
#else
    fprintf(out, ".intel_syntax noprefix\n\n");
#endif

#if defined(__APPLE__)
    fprintf(out, ".section __TEXT,__cstring,cstring_literals\n");
#else
    fprintf(out, ".section .rodata\n");
#endif

    fprintf(out, ".Lfmt_print_int:\n    .asciz \"%%lld\\n\"\n");
    fprintf(out, ".Lfmt_read_int:\n    .asciz \"%%lld\"\n");
    for (int i = 0; i < num_strs; i++) {
        fprintf(out, ".Lstr%d:\n    .asciz \"", i);
        write_escaped(out, str_values[i]);
        fprintf(out, "\"\n");
    }
    fprintf(out, "\n");

    fprintf(out, ".text\n");
    fprintf(out, ".globl %s\n", ASM_MAIN_LABEL);
    fprintf(out, "%s:\n", ASM_MAIN_LABEL);

#if TARGET_ARM64
    fprintf(out, "    stp x29, x30, [sp, -16]!\n");
    fprintf(out, "    mov x29, sp\n");
    if (stack_size > 0) fprintf(out, "    sub sp, sp, #%d\n", stack_size);

    emit_stmts(root);

    if (stack_size > 0) fprintf(out, "    add sp, sp, #%d\n", stack_size);
    fprintf(out, "    mov w0, #0\n");
    fprintf(out, "    ldp x29, x30, [sp], 16\n");
    fprintf(out, "    ret\n");
#else
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
    if (stack_size > 0) fprintf(out, "    sub rsp, %d\n", stack_size);

    emit_stmts(root);

    fprintf(out, "    xor eax, eax\n");
    fprintf(out, "    leave\n");
    fprintf(out, "    ret\n");
#endif

    fclose(out);
}
