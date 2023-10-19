#ifndef simscript_chunk_h
#define simscript_chunk_h

#include "common.h"
#include "value.h"

/**
 * @brief Holds what sort of instruction we are currently at
 *
 */
typedef enum {
    OP_CONSTANT,    // Produces a constant

    OP_NULL,
    OP_TRUE,
    OP_FALSE,

    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,

    OP_INCREMENT,
    OP_DECREMENT,

    OP_NOT,

    OP_NEGATE,
    OP_PRINT,
    OP_BREAK,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,   // Returns from the current function
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD
} OpCode;

/**
 * @brief Defining a chunk as a pointer to uint8
 *
 */
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk; 

/**
 * @brief Chunk constructor. Zero initializes all the fields in the struct.
 * @param chunk a pointer to a Chunk struct
 *
 */
void initChunk( VM* vm, Chunk* chunk );

/**
 * @brief Method to append a byte to the end of the chunk
 * @param chunk The chunk to append the byte to
 * @param byte The byte to append
 * @param line The line number where the instruction is written
 *
 */
void writeChunk( VM* vm, Chunk* chunk, uint8_t byte, int line );

/**
 * @brief Method to add a constant to the constants pool (array)
 * @param chunk The chunk to which the constant pool will be modified
 * @param value The Value that will be added to the constant pool
 * @return int The index value where the added value lives
 *
 */
int addConstant( VM* vm, Chunk* chunk, Value value );

/**
 * @brief Method to free the chunk pointer
 * @param chunk The chunk to free
 *
 */
void freeChunk( VM* vm, Chunk* chunk );

#endif
