#include <stdio.h>
#include <stdlib.h>

#include "reader.h"

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
