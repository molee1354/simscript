#include "string.h"
#include "../natives.h"

static Value lengthMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError(vm, "'length()' takes no argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjString* str = AS_STRING(args[0]);
    return NUMBER_VAL(str->length);
}

void defineStringMethods(VM* vm) {
    defineNative(vm, &vm->stringMethods, "length", lengthMethod);
}
