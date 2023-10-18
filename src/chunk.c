#include <stdlib.h>

#include "chunk.h"
#include "value.h"
#include "memory.h"
#include "vm.h"

void initChunk(VM* vm __attribute__((unused)), Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    /* initializing the line number array */
    chunk->lines = NULL;

    /* initialized as an address since the chunk struct does not 
    have it as a pointer */
    initValueArray(&chunk->constants);
}

void writeChunk(VM* vm, Chunk* chunk, uint8_t byte, int line) {
    // checking to see if array has enough capacity
    if (chunk->capacity < chunk->count+1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(vm, uint8_t,
                chunk->code,
                oldCapacity,
                chunk->capacity);

        // growing the line number array along with the code array
        chunk->lines = GROW_ARRAY(vm, int,
                chunk->lines,
                oldCapacity,
                chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(VM* vm, Chunk *chunk, Value value) {
    push(vm, value);
    writeValueArray(vm, &chunk->constants, value);

    // returning the index at which the value was added
    pop(vm);
    return chunk->constants.count - 1;
}

void freeChunk(VM* vm, Chunk* chunk) {
    FREE_ARRAY(vm, uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(vm, int, chunk->lines, chunk->capacity);
    freeValueArray(vm, &chunk->constants);
    initChunk(vm, chunk); // zero out the fields, so it's in an empty state
}
