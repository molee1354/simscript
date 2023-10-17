#ifndef simscript_memory_h
#define simscript_memory_h

#include "common.h"
#include "object.h"

/**
 * @brief Macro to allocate a new array on the heap
 *
 */
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type)*(count) )

/**
 * @brief Macro definition to free a pointer
 *
 */
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0);

/**
 * @brief Macro definition to double the capacity of the byte array
 *
 */
#define GROW_CAPACITY( capacity ) \
    ( (capacity) < 8 ? 8 : (capacity)*2 )

/**
 * @brief Macro definition to grow the capacity of the array
 *
 */
#define GROW_ARRAY( type, pointer, oldCount, newCount ) \
    (type*)reallocate( pointer, sizeof(type)*(oldCount), \
                       sizeof(type)*(newCount) )

/**
 * @brief Macro definition to free the dynamic array. Simply freeing 
 *  and resizing the array to a zero count. Wraps reallocate by passing
 *  in a zero as a new count.
 *
 */
#define FREE_ARRAY( type, pointer, oldCount ) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * @brief Method to reallocate and resize memory into a given size.
 * @param pointer pointer to the memory block to resize
 * @param oldSize old size
 * @param newSize new size
 *
 * @return void* a pointer to the newly sized block.
 * Will be casted to its new type
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

/**
 * @brief Method to mark an object
 *
 * @param object Object to mark
 */
void markObject(Obj* object);

/**
 * @brief Method to mark a value
 *
 * @param value Value to mark
 */
void markValue(Value value);

/**
 * @brief Garbace collection
 *
 */
void collectGarbage();

/**
 * @brief Method to free all the remaining objects
 */
void freeObjects();

#endif
