#ifndef simscript_SVM_h
#define simscript_SVM_h

typedef struct _vm VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

VM* initVM();

void freeVM(VM* vm);

InterpretResult interpret(VM* vm, const char* source);

#endif
