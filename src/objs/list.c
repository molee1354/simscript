#include "list.h"
#include "../natives.h"

/**
 * @brief Adding an element to the end of the list. O(n)
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value appendMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'append(value)' expects exactly one argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    appendList(vm, list, args[1]);
    return NULL_VAL;
}

/**
 * @brief Adding an element at index 0. O(n)
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value prependMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'prepend(value)' expects exactly one argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    appendList(vm, list, NULL_VAL);
    for (int i = list->items.count-1; i > 0; i--) {
        list->items.values[i] = list->items.values[i-1];
    }
    list->items.values[0] = args[1];
    return NULL_VAL;
}

/**
 * @brief Insert an element at a specified index. O(n)
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value insertMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError(vm, "'insert(value, index)' expects two arguments (%d provided).",
                argCount);
        return BAD_VAL;
    }
    if (!IS_NUMBER(args[1])){
        runtimeError(vm, "Wrong argument type for arg 'index' in method 'insert()'.");
    }
    ObjList* list = AS_LIST(args[0]);
    int index = AS_NUMBER(args[1]);
    if (index > list->items.count) {
        runtimeError(vm, "List index out of bounds (given %d, length %d).",
                index, list->items.count-1);
        return BAD_VAL;
    }
    appendList(vm, list, NULL_VAL);
    for (int i = list->items.count-1; i >= index; i--) {
        list->items.values[i] = list->items.values[i-1];
    }
    list->items.values[index] = args[2];
    return NULL_VAL;
}

/**
 * @brief Deleting an element at a specified index.
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value deleteMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'delete(index)' expects exactly one argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    int index = AS_NUMBER(args[1]);
    if (index > list->items.count) {
        runtimeError(vm, "List index out of bounds (given %d, length %d).",
                index, list->items.count-1);
        return BAD_VAL;
    }
    deleteFromIndexList(vm, list, index);
    return NULL_VAL;
}

static Value pushMethod(VM* vm, int argCount, Value* args) {
    prependMethod(vm, argCount, args);
    return NULL_VAL;
}

/**
 * @brief Pop value from list head. Deletes value as well.
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value popMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError(vm, "'pop()' expects no arguments (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    Value out = list->items.values[0];
    deleteFromIndexList(vm, list, 0);
    return out;
}

/**
 * @brief Same as prepend. O(n)
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value enqueueMethod(VM* vm, int argCount, Value* args) {
    prependMethod(vm, argCount, args);
    return NULL_VAL;
}

/**
 * @brief Dequeue value from list end. Deletes and returns value.
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value dequeueMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError(vm, "'pop()' expects no arguments (%d provided)",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    int outIdx = list->items.count-1;
    Value out = list->items.values[outIdx];
    deleteFromIndexList(vm, list, outIdx);
    return out;
}

/**
 * @brief If a given element is found in the list, returns the index, or returns
 * null. O(n)
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value findMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'find()' expects one argument (%d provided)",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    Value compare = args[1];
    for (int i = 0; i < list->items.count; i++) {
        if (valuesEqual(list->items.values[i], compare)) {
            return NUMBER_VAL(i);
        }
    }
    return NULL_VAL;
}

/**
 * @brief Returns true of given value exists in an array
 *
 * @param vm 
 * @param argCount 
 * @param args 
 * @return 
 */
static Value containsMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'find()' expects one argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    Value targ = args[1];
    for (int i = 0; i < list->items.count; i++) {
        if (valuesEqual(list->items.values[i], targ)) {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

static Value extendMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError(vm, "'extend(list)' expects one argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    ObjList* add = AS_LIST(args[1]);
    for (int i = 0; i < add->items.count; i++) {
        appendList(vm, list, add->items.values[i]);
    }
    return NULL_VAL;
}

static Value lengthMethod(VM* vm, int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError(vm, "'length()' expects exactly zero argument (%d provided).",
                argCount);
        return BAD_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    return NUMBER_VAL(list->items.count);
}

static Value reverseMethod(VM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "reverse() takes no arguments (%d provided).", argCount);
        return BAD_VAL;
    }

    ObjList *list = AS_LIST(args[0]);
    int listLength = list->items.count;

    for (int i = 0; i < listLength / 2; i++) {
        Value temp = list->items.values[i];
        list->items.values[i] = list->items.values[listLength - i - 1];
        list->items.values[listLength - i - 1] = temp;
    }
    return NULL_VAL;
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
}
