#include <stdio.h>
#include <stdlib.h>

void bad_write_int(long long value) {
    printf("%lld\n", value);
}

void bad_read_int(long long *out_value) {
    if (!out_value) {
        fprintf(stderr, "Runtime error: null destination for read()\n");
        exit(1);
    }

    fputs("input> ", stdout);
    fflush(stdout);

    if (scanf("%lld", out_value) != 1) {
        fprintf(stderr, "Runtime error: read() expected an integer input\n");
        exit(1);
    }
}
