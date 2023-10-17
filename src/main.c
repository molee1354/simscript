#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "read.h"
#include "vm.h"

static void repl() {
    puts("simscript v0.0.1");
    char line[1024];
    for (;;) {
        printf("\n>>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result==INTERPRET_COMPILE_ERROR) exit(65);
    if (result==INTERPRET_COMPILE_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    // init vm
    initVM();

    if (argc==1) {
        repl();
    } else if (argc==2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: hwin [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}
