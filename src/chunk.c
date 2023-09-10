#include <stdlib.h>

#include "chunk.h"
#include "value.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    /* initializing the line number array */
    chunk->lines = NULL;

    /* initialized as an address since the chunk struct does not 
    have it as a pointer */
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // checking to see if array has enough capacity
    if (chunk->capacity < chunk->count+1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t,
                chunk->code,
                oldCapacity,
                chunk->capacity);

        // growing the line number array along with the code array
        chunk->lines = GROW_ARRAY(int,
                chunk->lines,
                oldCapacity,
                chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(Chunk *chunk, Value value) {
    push(value);
    writeValueArray(&chunk->constants, value);

    // returning the index at which the value was added
    pop();
    return chunk->constants.count - 1;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk); // zero out the fields so it's in an empty state
}
