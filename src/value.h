#ifndef simscript_value_h
#define simscript_value_h

#include <string.h>

#include "common.h"
#include "SVM.h"

/**
 * @brief Struct to define the state shared by all Obj types
 *
 */
typedef struct Obj Obj;

/**
 * @brief Struct to define strings
 *
 */
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

typedef uint64_t Value;

#define SIGN_BIT            ( (uint64_t)0x8000000000000000 )
#define QNAN                ( (uint64_t)0x7ffc000000000000 )

#define TAG_NULL  1
#define TAG_FALSE 2
#define TAG_TRUE  3

#define IS_BOOL(value)      ( ((value) | 1) == TRUE_VAL)
#define IS_NULL(value)      ( (value) == NULL_VAL )
#define IS_NUMBER(value)    ( ((value) & QNAN) != QNAN )
#define IS_OBJ(value) \
    ( ((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT) )

#define AS_BOOL(value)      ( (value) == TRUE_VAL )
#define AS_NUMBER(value)    valueToNum(value)
#define AS_OBJ(value) \
    ( (Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)) )

#define BOOL_VAL(b)         ( (b) ? TRUE_VAL : FALSE_VAL )
#define TRUE_VAL            ( (Value)(uint64_t)(QNAN | TAG_TRUE) )
#define FALSE_VAL           ( (Value)(uint64_t)(QNAN | TAG_FALSE) )
#define NULL_VAL            ( (Value)(uint64_t)(QNAN | TAG_NULL) )
#define NUMBER_VAL(num)     numToValue(num)
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

/**
 * @brief Enum to hold the different types that are supported
 *
 */
typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

/**
 * @brief Defining Value with a union for double and bool
 *
 */
typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

/**
 * @brief Value type checking macros
 *
 */
#define IS_BOOL(value)   ( (value).type == VAL_BOOL )
#define IS_NULL(value)   ( (value).type == VAL_NULL )
#define IS_NUMBER(value) ( (value).type == VAL_NUMBER )
#define IS_OBJ(value)    ( (value).type == VAL_OBJ )

/**
 * @brief Unpacking the values into the C values
 *
 */
#define AS_BOOL(value) ( (value).as.boolean )
#define AS_NUMBER(value) ( (value).as.number )
#define AS_OBJ(value) ( (value).as.obj )

/**
 * @brief Converting a native C value into the language
 * 
 */
#define BOOL_VAL(value)   ( (Value){VAL_BOOL, {.boolean = value}} )
#define NULL_VAL          ( (Value){VAL_NULL, {.number = 0}} )
#define NUMBER_VAL(value) ( (Value){VAL_NUMBER, {.number = value}} )
#define OBJ_VAL(object)   ( (Value){VAL_OBJ, {.obj = (Obj*)object}} )

#endif // for NAN_BOXING

/**
 * @brief The dynamic array holding the constant pool of Values.
 *
 */
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

/**
 * @brief Method to check if the two values are equal
 *
 */
bool valuesEqual(Value a, Value b);

/**
 * @brief Method to initialize the value array
 * @param array Pointer to value array
 *
 */
void initValueArray(ValueArray* array);

/**
 * @brief Method to write a value to the value array
 * @param array Pointer to value array
 * @param value Value to write
 *
 */
void writeValueArray(VM* vm, ValueArray* array, Value value);
    
/**
 * @brief Method to free the value array
 * @param array Pointer to value array
 *
 */
void freeValueArray(VM* vm, ValueArray* array);

/**
 * @brief Method to print a given value of type Value
 * @param value Value to print
 *
 */
void printValue(Value value);
    
#endif
