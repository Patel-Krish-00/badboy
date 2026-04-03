#include <stdio.h>
#include "ast.h"

void generate_asm(ASTNode* root, const char* filename) {
    FILE* out = fopen(filename, "w");

    fprintf(out, ".section __TEXT,__text\n");
    fprintf(out, ".global _main\n");
    fprintf(out, ".extern _printf\n\n");

    fprintf(out, "_main:\n");

    // Load format string
    fprintf(out, "    adrp x0, fmt@PAGE\n");
    fprintf(out, "    add x0, x0, fmt@PAGEOFF\n");

    // Load message string
    fprintf(out, "    adrp x1, msg@PAGE\n");
    fprintf(out, "    add x1, x1, msg@PAGEOFF\n");

    // Call printf(fmt, msg)
    fprintf(out, "    bl _printf\n");

    // return 0
    fprintf(out, "    mov x0, #0\n");
    fprintf(out, "    ret\n\n");

    // Data section
    fprintf(out, ".section __TEXT,__cstring\n");

    fprintf(out, "fmt:\n");
    fprintf(out, "    .asciz \"%%s\\n\"\n");

    fprintf(out, "msg:\n");
    fprintf(out, "    .asciz \"%s\"\n", root->value);

    fclose(out);
}