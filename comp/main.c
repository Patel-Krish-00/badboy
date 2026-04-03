#include <stdio.h>
#include <stdlib.h>
#include "bad.h"
#include "ast.h"

static char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc(size + 1);
    if (!buffer) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./bad <file.bad>\n");
        return 1;
    }

    char *source = read_file(argv[1]);

    init_lexer(source);
    ASTNode* root = parse();

    int status = 0;
#if defined(__linux__) && defined(__x86_64__)
    /* Generate x86_64 Linux assembly directly — no gcc/clang as C compiler */
    generate_asm(root, "out.s");
    printf("Generated assembly: out.s\n");

    /* Assemble and link (clang used only as assembler/linker, not as C compiler) */
    status = system("clang -nostdlib out.s -o out");
#else
    /* Fallback for non-Linux/x86_64 hosts: emit portable C and compile it */
    generate_c(root, "out.c");
    printf("Generated C: out.c\n");
    status = system("clang out.c -o out");
#endif
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        free(source);
        return 1;
    }

    printf("Running output:\n");
    int run = system("./out");
    if (run != 0) fprintf(stderr, "Execution failed\n");

    free(source);
    return 0;
}
