#ifndef _vm_h
#define _vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

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
typedef struct {
    CallFrame frames[FRAMES_MAX]; // each callframe has its own ip and
                                  // pointer to ObjFunction
    int frameCount; // current height of the frames stack

    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;            // hash table to hold global variables
    Table strings;            // every string that's created
    ObjString* initString;
    ObjUpvalue* openUpvalues; // upvalue array

    size_t bytesAllocated;
    size_t nextGC;
    Obj* objects;             // vm stores the head of the objects list
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

/**
 * @brief VM Constructor
 *
 */
void initVM();

/**
 * @brief VM Destructor
 *
 */
void freeVM();

/**
 * @brief Interpreting the instructions in the chunk
 * @param chunk The pointer to the chunk to interpret
 *
 */
InterpretResult interpret(const char* source);

/**
 * @brief Pushing a value into the vm stack
 * @param value Value to push into the stack
 *
 */
void push(Value value);

/**
 * @brief Popping the topmost value out out of the vm stack
 * @return Value The value that was at the top of the stack
 *
 */
Value pop();

#endif
