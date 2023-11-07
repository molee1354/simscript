#include <stdio.h>

#include "../natives.h"
#include "error.h"

static Value errorMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'error(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    fprintf(stderr, "\033[0;31m");
    for (int i = 0; i < argCount; i++) {
        printValue(stderr, args[i]);
        fprintf(stderr," ");
    }
    fprintf(stderr, "\033[0m\n");
    return NULL_VAL;
}

static Value errorlnMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'errorln(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    fprintf(stderr, "\033[0;31m");
    for (int i = 0; i < argCount; i++) {
        printValue(stderr, args[i]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\033[0m");
    return NULL_VAL;
}

ObjModule* initLib_Error(VM* vm) {
    ObjString* name = copyString(vm, "Error", 5);
    push(vm, OBJ_VAL(name));
    ObjModule* errorLib = newModule(vm, name);
    push(vm, OBJ_VAL(errorLib));
    defineNative(vm, &errorLib->values, "print", errorMethod);
    defineNative(vm, &errorLib->values, "println", errorlnMethod);
    pop(vm);
    pop(vm);
    return errorLib;
}
