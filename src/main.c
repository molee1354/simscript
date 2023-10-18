#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "read.h"
#include "vm.h"

#define VERSION "0.0.6"
#define TIME    "Oct 19 2023, 07:43"

#ifdef _WIN32
#define PLATFORM "Windows"
#else
#define PLATFORM "Linux"
#endif

static void repl() {
    printf("Simscript REPL v%s (%s) for %s\n", VERSION, TIME, PLATFORM);
    puts("Enter \"exit\" to exit.");
    char line[1024];
    for (;;) {
        printf("\n>>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        if (!strncmp(line, "exit", 4)) {
            exit(0);
        }

        interpret(line);
    }
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result==INTERPRET_COMPILE_ERROR) exit(65);
    if (result==INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    // init vm
    initVM();

    if (argc==1) {
        repl();
    } else if (argc==2) {
        if (!strcmp(argv[1], "--version")) {
            printf("Simscript %s\n\n", VERSION);
        } else {
            runFile(argv[1]);
        }
    } else {
        fprintf(stderr, "Usage: simscript [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}
