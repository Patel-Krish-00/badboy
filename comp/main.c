#include <stdio.h>
#include <stdlib.h>
#include "bad.h"
#include "ast.h"

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        printf("Error: Could not open file\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);

    if (!buffer) {
        printf("Memory allocation failed\n");
        exit(1);
    }

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

    // 🔥 Generate assembly
    generate_asm(root, "out.s");

    printf("Generated assembly: out.s\n");

    // 🔥 Compile assembly → executable
    int compile_status = system("clang out.s -o out");

    if (compile_status != 0) {
        printf("Assembly compilation failed\n");
        return 1;
    }

    printf("Running output:\n");

    // 🔥 Run executable
    int run_status = system("./out");

    if (run_status != 0) {
        printf("Execution failed\n");
    }

    free(root);   // free AST
    free(source); // free file buffer

    return 0;
}