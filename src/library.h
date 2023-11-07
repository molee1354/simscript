#ifndef simscript_library_h
#define simscript_library_h

#include "vm.h"

typedef ObjModule* (*StandardLib)(VM* vm);
//typedef Value (*NativeFn)(VM* vm, int argCount, Value* args);

typedef struct {
    char* name;
    StandardLib libInitFunc;
} StdLib;

ObjModule* importStdLib(VM* vm, int index);

int getStdLib(VM* vm, const char* name, int length);

#endif
