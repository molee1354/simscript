#ifndef simscript_table_h
#define simscript_table_h

#include "common.h"
#include "value.h"

/**
 * @brief Struct to define an entry in the hash table
 */
typedef struct {
    ObjString* key;
    Value value;
} Entry;

/**
 * @brief Struct to define a hash table. Ratio of count to capacity is
 * the load factor of the table.
 */
typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

/**
 * @brief Table constructor
 *
 * @param table A pointer to a table struct
 */
void initTable(Table* table);

/**
 * @brief Method to free the table
 *
 * @param table The pointer to the table to free
 */
void freeTable(VM* vm, Table* table);

/**
 * @brief Method to get a value from a table given a key
 *
 * @param table Table to perform the lookup
 * @param key The key to get the value from
 * @param value The pointer to the value that will hold the value
 * @return True if value exists
 */
bool tableGet(VM* vm, Table* table, ObjString* key, Value* value);

/**
 * @brief Method to add the given key-value pair to the hast table
 *
 * @param table Table to hold the key-value pair
 * @param key Key to call value
 * @param value Value called by key
 * @return True if a new entry was added
 */
bool tableSet(VM* vm, Table* table, ObjString* key, Value value);

/**
 * @brief Method to delete a key from the table
 *
 * @param table Table to delete the key from
 * @param key The key to delete
 * @return True if key existed
 */
bool tableDelete(VM* vm, Table* table, ObjString* key);

/**
 * @brief Method to add all the entries found in one table to another
 *
 * @param from Source table
 * @param to Destination Table
 */
void tableAddAll(VM* vm, Table* from, Table* to);

/**
 * @brief Method to find string values at a table.
 *
 * @param table Table from which to find the string values
 * @param chars Target string to find. Not an ObjString* struct
 * @param length Target string length
 * @param hash Hash key for comparison
 * @return ObjString* Returns the matching key value
 */
ObjString* tableFindString(Table* table, const char* chars, int length,
                           uint32_t hash);

/**
 * @brief Method to remove the dangling string pointers in the table
 *
 * @param table The table to free string pointers from
 */
void tableRemoveWhite(VM* vm, Table* table);

/**
 * @brief Method to mark a table 
 *
 * @param table The table to mark
 */
void markTable(VM* vm, Table* table);

#endif
