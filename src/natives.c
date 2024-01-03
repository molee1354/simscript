#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "natives.h"

// check for platform
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * @brief Defining the native "clock()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value The elapsed time since the program started running
 */
static Value clockNative(VM* vm, int argCount, Value* args ) {
    UNUSED(vm);
    UNUSED(argCount);
    UNUSED(args);

    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

/**
 * @brief Defining the native "sleep()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value Null val.
 */
static Value sleepNative(VM* vm, int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError(vm, "Too many arguments provided : %d", argCount);
        return BAD_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "Incorrect argument type.");
        return BAD_VAL;
    }
    int waitFor = (int)AS_NUMBER(args[0]);

    /* clock_t timeStart = clock();
    while (clock() < timeStart + waitFor)
        ; */
#ifdef _WIN32
    Sleep(waitFor*1000);
#else
    sleep(waitFor);
#endif
    return NULL_VAL;
}

/**
 * @brief Method to exit the code with an exitcode
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value No return value
 */
static Value exitNative(VM* vm, int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError(vm, "Too many arguments provided : %d", argCount);
        return BAD_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "Incorrect argument type.");
        return BAD_VAL;
    }
    int exitCode = (int)AS_NUMBER(args[0]);
    printf("Program exit with exitcode %d\n", exitCode);
    exit(exitCode);
    return NULL_VAL;
}

/**
 * @brief Defining the native "puts()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value NULL_VAL
 */
static Value putsNative(VM* vm, int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError(vm, "Too many arguments provided : %d", argCount);
        return BAD_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError(vm, "Incorrect argument type.");
        return BAD_VAL;
    }
    printf("%s\n", AS_CSTRING(args[0]));
    return NULL_VAL;
}

/**
 * @brief Defining the native "system()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value NULL_VAL
 */
static Value systemNative(VM* vm, int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError(vm, "Too many arguments provided : %d", argCount);
        return BAD_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError(vm, "Incorrect argument type.");
        return BAD_VAL;
    }
    system(AS_CSTRING(args[0]));
    return NULL_VAL;
}

void defineNative(VM* vm, Table* table, const char* name, NativeFn function) {
//    push( vm, OBJ_VAL(copyString(vm, name, (int)strlen(name))) );
//    push( vm, OBJ_VAL(newNative(vm, function)) );
//    tableSet(vm, table, AS_STRING(vm->stack[0]), OBJ_VAL(vm->stack[1]));
    ObjNative* native = newNative(vm, function);
    push(vm, OBJ_VAL(native));
    ObjString* fname = copyString(vm, name, (int)strlen(name));
    push(vm, OBJ_VAL(fname));
    tableSet(vm, table, fname, OBJ_VAL(native));
    pop(vm);
    pop(vm);
}

void defineNatives(VM* vm) {
    // defining native functions
    defineNative(vm, &vm->globals, "clock", clockNative);
    defineNative(vm, &vm->globals, "sleep", sleepNative);
    defineNative(vm, &vm->globals, "puts", putsNative);
    defineNative(vm, &vm->globals, "exit", exitNative);
    defineNative(vm, &vm->globals, "system", systemNative);
}

