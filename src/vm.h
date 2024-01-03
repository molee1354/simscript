#ifndef simscript_vm_h
#define simscript_vm_h

#include "compiler.h"
#include "value.h"
#include "object.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * @brief Struct to define the callframe of a function
 *
 */
typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

/**
 * @brief Struct to define the VM that runs the bytecode
 *
 */
struct _vm {
    Compiler* compiler;
    CallFrame frames[FRAMES_MAX]; // each callframe has its own ip and
                                  // pointer to ObjFunction
    bool repl;
    int frameCount; // current height of the frames stack

    Value stack[STACK_MAX];
    Value* stackTop;
    ObjString* initString;
    ObjUpvalue* openUpvalues; // upvalue array

    Table globals;            // hash table to hold global variables
    Table strings;            // every string that's created
    Table listMethods;        // list methods
    Table stringMethods;      // string methods

    ObjModule* lastModule;    // modules
    Table modules;            // 

    size_t bytesAllocated;
    size_t nextGC;
    Obj* objects;             // vm stores the head of the objects list
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
};

// Runtime error function declaration
void runtimeError(VM* vm, const char* format, ...);

// Runtime warning function declaration
void runtimeWarning(VM* vm, const char* format, ...);

/**
 * @brief Pushing a value into the vm stack
 * @param value Value to push into the stack
 *
 */
void push(VM* vm, Value value);

/**
 * @brief Popping the topmost value out out of the vm stack
 * @return Value The value that was at the top of the stack
 *
 */
Value pop(VM* vm);

#endif
