#ifndef simscript_SVM_h
#define simscript_SVM_h

#include "common.h"

typedef struct _vm VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

VM* initVM(bool repl);

void freeVM(VM* vm);

InterpretResult interpret(VM* vm, const char* source);

#endif
