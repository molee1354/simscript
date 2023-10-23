#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "object.h"

void disassembleChunk(Chunk *chunk, const char *name) {
    printf("== %s ==\n", name);

    for (int offset=0; offset < chunk->count; ) {
        offset = disassembleInstruction(chunk, offset);
    }
}

/**
 * @brief Method to debug a constant instruction
 *
 * @param name Name of the current instruction
 * @param chunk The current chunk to write on
 * @param offset The current offset
 * @return int offset increment
 */
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset+1];
    printf("\033[0;32m%-16s\033[0m %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    /* +2 to the offset since OP_CONST is two bytes: 1 for opcode and 1 for
    operand */
    return offset+2;
}

static int invokeInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset+1];
    uint8_t argCount = chunk->code[offset+2];
    printf("\033[0;32m%-16s\033[0m (%d args) %4d '", name, argCount, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset+3;
}

/**
 * @brief Display method for a simple instruction
 * @param name Name of the current instruction
 * @param offset The current offset
 *
 * @return static int The newly updated offset
 * 
 */
static int simpleInstruction(const char* name, int offset) {
    printf("\033[0;32m%s\033[0m\n", name);
    return offset+1;
}

/**
 * @brief Display method for byte instructions. Used for showing local
 * variables and their slot numbers.
 *
 * @param name Name of the operation
 * @param chunk The current chunk within bytecode
 * @param offset The current location in code
 * @return static int The current offset of the code
 */
static int byteInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t slot = chunk->code[offset+1];
    printf("\033[0;32m%-16s\033[0m %4d\n", name, slot);
    return offset+2;
}

/**
 * @brief Method to disassmeble 16-bit operand jump instructions.
 *
 * @param name The name of the operand
 * @param sign The sign of the operand
 * @param chunk The current chunk
 * @param offset The current location of the code
 * @return static int The current offset of the code
 */
static int jumpInstruction(const char* name, int sign, Chunk* chunk,
                           int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset+1] << 8);
    jump |= chunk->code[offset+2];
    printf("\033[0;32m%-16s\033[0m %4d -> %d\n", name, offset,
            offset+3+sign*jump);
    return offset+3;
}

int disassembleInstruction(Chunk *chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 &&
        chunk->lines[offset] == chunk->lines[offset-1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];

    switch (instruction) {
        case OP_NULL:
            return simpleInstruction("OP_NULL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL", chunk, offset);
        case OP_GET_UPVALUE:
            return byteInstruction("OP_GET_UPVALUE", chunk, offset);
        case OP_SET_UPVALUE:
            return byteInstruction("OP_SET_UPVALUE", chunk, offset);

        case OP_GET_PROPERTY:
            return constantInstruction("OP_GET_PROPERTY", chunk, offset);
        case OP_SET_PROPERTY:
            return constantInstruction("OP_SET_PROPERTY", chunk, offset);
        case OP_GET_SUPER:
            return constantInstruction("OP_GET_SUPER", chunk, offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);

        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_MOD:
            return simpleInstruction("OP_MOD", offset);

        case OP_INCREMENT:
            return simpleInstruction("OP_INCREMENT", offset);
        case OP_DECREMENT:
            return simpleInstruction("OP_DECREMENT", offset);

        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);

        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);
        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);
        case OP_INVOKE:
            return invokeInstruction("OP_INVOKE", chunk, offset);
        case OP_SUPER_INVOKE:
            return invokeInstruction("OP_SUPER_INVOKE", chunk, offset);
        case OP_CLOSURE: {
            offset++;
            uint8_t constant = chunk->code[offset++];
            printf("%-16s %4d ", "OP_CLOSURE", constant);
            printValue(chunk->constants.values[constant]);
            printf("\n");
            ObjFunction* function = AS_FUNCTION(
                    chunk->constants.values[constant]);
            for (int j=0; j < function->upvalueCount; j++) {
                int isLocal = chunk->code[offset++];
                int index = chunk->code[offset++];
                printf("%04d      |                     %s %d\n",
                       offset - 2, isLocal ? "local" : "upvalue", index);
            }
            return offset;
        }
        case OP_CLOSE_UPVALUE:
            return simpleInstruction("OP_CLOSE_UPVALUE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CLASS:
            return constantInstruction("OP_CLASS", chunk, offset);
        case OP_END_CLASS:
            return constantInstruction("OP_END_CLASS", chunk, offset);
        case OP_INHERIT:
            return simpleInstruction("OP_INHERIT", offset);
        case OP_METHOD:
            return constantInstruction("OP_METHOD", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset+1;
    }
}

