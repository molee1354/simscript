#ifndef _debug_h
#define _debug_h

#include "chunk.h"

/**
 * @brief Method to disassemble all the instructions in the entire chunk
 * @param chunk Chunk to disassemble
 * @param name String name of chunk to disassemble
 *
 */
void disassembleChunk(Chunk* chunk, const char* name);

/**
 * @brief Method to disassemble all the instructions at the given bytecode
 * offset
 * @param chunk Chunk to disassemble
 * @param offset the current offset
 *
 */
int disassembleInstruction(Chunk* chunk, int offset);

#endif
