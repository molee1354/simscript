#ifndef simscript_read_h
#define simscript_read_h

#include "common.h"
#include "vm.h"

#define PATHSEP_STRLEN   1

#ifdef _WIN32
#define PATHSEP         '\\'
#define PATHSEP_STR     "\\"
#else
#define PATHSEP         '/'
#define PATHSEP_STR     "/"
#endif

ObjString* dirName(VM* vm, char* path, int len);

ObjString* getDirectory(VM* vm, char* source);

char* readFile_VM(VM* vm, const char* path);

char* readFile(const char* path);

bool validPath(char* directory, char* path, char* ret);

#endif
