#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    puts("simscript v0.1.0");
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

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file==NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // finding the file size off of byte offset
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // allocating enough memory to hold the file size + 1
    char* buffer = (char*)malloc(fileSize+3);
    if (buffer==NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // adding a fake local scope
    buffer[0] = '{';
    size_t bytesRead = fread(buffer+sizeof(char), sizeof(char), fileSize, file);
    if(bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead+1] = '}';
    buffer[bytesRead+2] = '\0';

    fclose(file);
    return buffer;
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
