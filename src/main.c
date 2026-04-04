#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
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

static void ensure_build_dir(void) {
#if defined(_WIN32)
    if (_mkdir("build") != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create build directory\n");
        exit(1);
    }
#else
    if (mkdir("build", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create build directory\n");
        exit(1);
    }
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./bad <file.bad>\n");
        return 1;
    }

    char *source = read_file(argv[1]);

    init_lexer(source);
    ASTNode* root = parse();
    ensure_build_dir();

    int status = 0;
    const char *artifact = "build/out.s";
    const char *stage = "Assembly";

    /* Always emit assembly, with platform-specific ABI handled in codegen_asm.c */
    generate_asm(root, "build/out.s");
    printf("Generated assembly: build/out.s\n");

#if defined(_WIN32)
    status = system("clang build/out.s src/runtime.c -o build/out.exe");
#else
    status = system("clang build/out.s src/runtime.c -o build/out");
#endif
    if (status != 0) {
        fprintf(stderr, "%s failed (status=%d). Generated artifact: %s\n",
                stage, status, artifact ? artifact : "(unknown)");
        free(source);
        return 1;
    }

    printf("Running output:\n");
#if defined(_WIN32)
    int run = system("build/out.exe");
#else
    int run = system("./build/out");
#endif
    if (run != 0) fprintf(stderr, "Execution failed\n");

    free(source);
    return 0;
}
