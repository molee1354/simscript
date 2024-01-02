#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "read.h"

ObjString* dirName(VM* vm, char* path, int len) {

#define IS_PATHSEP(c) (c == PATHSEP)

    if (!len) {
        return copyString(vm, ".", 1);
    }

    char *sep = path + len;

    /* trailing slashes */
    while (sep != path) {
        if (0 == IS_PATHSEP(*sep))
            break;
        sep--;
    }

    /* first found */
    while (sep != path) {
        if (IS_PATHSEP(*sep))
            break;
        sep--;
    }

    /* trim again */
    while (sep != path) {
        if (0 == IS_PATHSEP(*sep))
            break;
        sep--;
    }

    if (sep == path && !IS_PATHSEP(*sep)) {
        return copyString(vm, ".", 1);
    }

    len = sep - path + 1;
#undef IS_PATHSEP

    return copyString(vm, path, len);
}

bool validPath(char *directory, char *path, char *ret) {
    char buf[PATHLEN]; 
    if (*path == PATHSEP) {
        snprintf(buf, PATHLEN, "%s", path);
    } else {
        snprintf(buf, PATHLEN, "%s%c%s", directory, PATHSEP, path);
    }

#ifdef _WIN32
    _fullpath(ret, buf, PATH_MAX);
#else
    if (realpath(buf, ret) == NULL) {
        return false;
    }
#endif
    return true;
}

ObjString* getDirectory(VM* vm, char* source) {
    int len = strlen(source);
    if (vm->repl || len < 4 || source[len - 3] != '.') {
        source = "";
    }

    char res[PATHLEN];
    if (!validPath(".", source, res)) {
        runtimeError(vm, "Unable to resolve path '%s'", source);
        exit(1);
    }

    if (vm->repl) {
        return copyString(vm, res, strlen(res));
    }

    return dirName(vm, res, strlen(res));
}


char* readFile_VM(VM* vm, const char* path) {
    FILE* file = fopen(path, "rb");
    if (file==NULL) {
        return NULL;
    }

    // finding the file size off of byte offset
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // allocating enough memory to hold the file size + 1
    char* buffer = ALLOCATE(vm, char, fileSize+1);
    if (buffer==NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // adding a fake local scope
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if(bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file==NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // finding the file size off of byte offset
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // allocating enough memory to hold the file size + 1
    char* buffer = (char*)malloc(fileSize+3);
    if (buffer==NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // adding a fake local scope
    buffer[0] = '{';
    size_t bytesRead = fread(buffer+sizeof(char), sizeof(char), fileSize, file);
    if(bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead+1] = '}';
    buffer[bytesRead+2] = '\0';

    fclose(file);
    return buffer;
}
