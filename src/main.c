#include "stdio.h"

int main(int argc, char *argv[]) {
    printf("Hello from C built with Zig\n\n");

    printf("Passed arguments:\n");
    for (size_t i = 0; i < argc; i++) {
        printf("%i\t%s\n", i, argv[i]);
    }

    return 0;
}