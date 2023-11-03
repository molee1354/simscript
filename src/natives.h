#ifndef simscript_natives_h
#define simscript_natives_h

#include "common.h"
#include "vm.h"

/**
 * @brief Method to define new native functions. It takes a pointer to a C
 * function and the name it will be known in the language implementation
 *
 * @param name Name of native function
 * @param function Pointer to C function
 */
void defineNative(VM* vm, Table* table, const char* name, NativeFn function);

/**
 * @brief Function to define all the native functions
 *
 * @param vm Pointer to the virtual current virtual machine instance
 */
void defineNatives(VM* vm);

#endif
