#include "list.h"
#include "natives.h"
#include "object.h"
#include "value.h"

static Value appendMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'append()' expects exactly one argument (%d provided)",
                argCount);
        return NULL_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    appendList(vm, list, args[1]);
    return OKAY_VAL;
}

static Value prependMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'prepend()' expects exactly one argument (%d provided)",
                argCount);
        return NULL_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    appendList(vm, list, NULL_VAL);
    for (int i = list->items.count-1; i > 0; i--) {
        list->items.values[i] = list->items.values[i-1];
    }
    list->items.values[0] = args[1];
    return OKAY_VAL;
}

static Value insertMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError(vm, "'insert()' expects two arguments (element, index. %d provided)",
                argCount);
        return NULL_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    int index = AS_NUMBER(args[1]);
    appendList(vm, list, NULL_VAL);
    for (int i = list->items.count-1; i >= index; i--) {
        list->items.values[i] = list->items.values[i-1];
    }
    list->items.values[index] = args[2];
    return OKAY_VAL;
}

static Value deleteMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value pushMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value popMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value enqueueMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value dequeueMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value findMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value containsMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value extendMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value toStringMethod(VM* vm, int argCount, Value* args) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);
    return OKAY_VAL;
}

static Value lengthMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError(vm, "'length()' expects exactly zero argument (%d provided)",
                argCount);
        return NULL_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    return NUMBER_VAL(list->items.count);
}

static Value reverseMethod(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "reverse() takes no arguments (%d provided)", argCount);
        return NULL_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    int listLength = list->items.count;

    for (int i = 0; i < listLength / 2; i++) {
        Value temp = list->items.values[i];
        list->items.values[i] = list->items.values[listLength - i - 1];
        list->items.values[listLength - i - 1] = temp;
    }
    return OKAY_VAL;
}

void defineListMethods(VM* vm) {
    defineNative(vm, &vm->listMethods, "append", appendMethod);
    defineNative(vm, &vm->listMethods, "prepend", prependMethod);
    defineNative(vm, &vm->listMethods, "length", lengthMethod);
    defineNative(vm, &vm->listMethods, "reverse", reverseMethod);

    defineNative(vm, &vm->listMethods, "contains", containsMethod);
    defineNative(vm, &vm->listMethods, "find", findMethod);
    defineNative(vm, &vm->listMethods, "delete", deleteMethod);
    defineNative(vm, &vm->listMethods, "insert", insertMethod);
    defineNative(vm, &vm->listMethods, "push", pushMethod);
    defineNative(vm, &vm->listMethods, "pop", popMethod);
    defineNative(vm, &vm->listMethods, "enqueue", enqueueMethod);
    defineNative(vm, &vm->listMethods, "dequeue", dequeueMethod);
    defineNative(vm, &vm->listMethods, "extend", extendMethod);
    defineNative(vm, &vm->listMethods, "toString", toStringMethod);
}
