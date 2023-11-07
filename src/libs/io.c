#include <stdio.h>

#include "io.h"
#include "../natives.h"

static Value printMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'print(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    for (int i = 0; i < argCount; i++) {
        printValue(args[i]);
        printf(" ");
    }
    puts("");
    return NULL_VAL;
}
static Value printlnMethod(VM* vm, int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError(vm, "'println(value, ...)' takes at least one argument (%d provided)",
                     argCount);
        return INTERPRET_RUNTIME_ERROR;
    }
    for (int i = 0; i < argCount; i++) {
        printValue(args[i]);
        printf("\n");
    }
    return NULL_VAL;
}

ObjModule* initLib_IO(VM* vm) {
    ObjString* name = copyString(vm, "IO", 2);
    push(vm, OBJ_VAL(name));
    ObjModule* ioLib = newModule(vm, name);
    push(vm, OBJ_VAL(ioLib));
    defineNative(vm, &ioLib->values, "print", printMethod);
    defineNative(vm, &ioLib->values, "println", printlnMethod);
    pop(vm);
    pop(vm);
    return ioLib;
}
