#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "../natives.h"
#include "../memory.h"

static Value printMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'IO.print(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    for (int i = 0; i < argCount; i++) {
        printValue(stdout, args[i]);
        printf(" ");
    }
    return NULL_VAL;
}

static Value printlnMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'IO.println(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    for (int i = 0; i < argCount; i++) {
        printValue(stdout, args[i]);
        printf(" ");
    }
    printf("\n");
    return NULL_VAL;
}

static Value inputMethod(VM* vm, int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError(vm, "'IO.input(prompt)' expects at most 1 argument. (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    if (argCount != 0) {
        Value prompt = args[0];
        if (!IS_STRING(prompt)) {
            runtimeError(vm, "'IO.input(prompt) takes a string-type argument.");
            return BAD_VAL;
        }
        printf("%s", AS_CSTRING(prompt));
    }

    uint64_t bufSize = 128;
    char* line = ALLOCATE(vm, char, bufSize);
    if (line == NULL) {
        runtimeError(vm, "'IO.input(prompt)' failed memory allocation.");
        return BAD_VAL;
    }
    int currentChar = EOF;
    uint64_t length = 0;
    while ( (currentChar = getchar()) != '\n' && currentChar != EOF) {
        line[length++] = (char)currentChar;

        if (length+1 == bufSize) {
            int oldSize = bufSize;
            bufSize = GROW_CAPACITY(bufSize);
            line = GROW_ARRAY(vm, char, line, oldSize, bufSize);
            if (line == NULL) {
                fprintf(stderr, "'IO.input(prompt)' failed memory allocation.");
                exit(71);
            }
        }
    }
    if (length != bufSize) {
        line = GROW_ARRAY(vm, char, line, bufSize, length+1);
    }
    line[length] = '\0';
    return OBJ_VAL(takeString(vm, line, length));
}

ObjModule* initLib_IO(VM* vm) {
    ObjString* name = copyString(vm, "IO", 2);
    push(vm, OBJ_VAL(name));
    ObjModule* ioLib = newModule(vm, name);
    push(vm, OBJ_VAL(ioLib));
    defineNative(vm, &ioLib->values, "print", printMethod);
    defineNative(vm, &ioLib->values, "println", printlnMethod);
    defineNative(vm, &ioLib->values, "input", inputMethod);
    pop(vm);
    pop(vm);
    return ioLib;
}
