#ifndef simscript_object_h
#define simscript_object_h

#include "SVM.h"
#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

/**
 * @brief Macro to extract the type tag
 *
 */
#define OBJ_TYPE(value) ( AS_OBJ(value)->type )

/**
 * @brief Macro to check if a value is of bound-method type
 *
 */
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)

/**
 * @brief Macro to check if a value is of class type
 *
 */
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)

/**
 * @brief Macro to check if a value is of closure type
 *
 */
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)

/**
 * @brief Macro to check if a value is of function type
 *
 */ 
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)

/**
 * @brief Macro to check if a value is of instance type
 *
 */
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)

/**
 * @brief Macro to check if a value is of native function type
 *
 */ 
#define IS_NATIVE(value)   isObjType(value, OBJ_NATIVE)

/**
 * @brief Macro to check if a value is of string type
 *
 */ 
#define IS_STRING(value)   isObjType(value, OBJ_STRING)

/**
 * @brief Macro to convert into a bound-method object
 *
 */ 
#define AS_BOUND_METHOD(value)       ( (ObjBoundMethod*)AS_OBJ(value) )

/**
 * @brief Macro to convert into class object
 *
 */ 
#define AS_CLASS(value)       ( (ObjClass*)AS_OBJ(value) )

/**
 * @brief Macro to convert into closure object
 *
 */ 
#define AS_CLOSURE(value)       ( (ObjClosure*)AS_OBJ(value) )

/**
 * @brief Macro to convert into function object
 *
 */ 
#define AS_FUNCTION(value)       ( (ObjFunction*)AS_OBJ(value) )

#define AS_INSTANCE(value)       ( (ObjInstance*)AS_OBJ(value) )

/**
 * @brief Macro to convert into a native function object
 *
 */
#define AS_NATIVE(value) \
    ( ((ObjNative*)AS_OBJ(value))->function )

/**
 * @brief Macro to convert into string implementation
 *
 */ 
#define AS_STRING(value)       ( (ObjString*)AS_OBJ(value) )

/**
 * @brief Macro to convert into C-strings
 *
 */ 
#define AS_CSTRING(value)      ( ((ObjString*)AS_OBJ(value))->chars )

/**
 * @brief Type tags
 *
 */
typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

/**
 * @class Obj
 * @brief Object wrapper to implement struct inheritance
 *
 */
struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

/**
 * @brief Enum to hold the different types of functions
 *
 */
typedef enum {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT,
} FunctionType;

/**
 * @class ObjFunction
 * @brief Defining functions as first class.
 */
typedef struct {
    Obj obj;
    int params;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
    FunctionType type;
} ObjFunction;

/**
 * @brief Declare NativeFn as a function pointer that has a return type of
 * Value
 *
 */
typedef Value (*NativeFn)(VM* vm, int argCount, Value* args);

/**
 * @class ObjNative
 * @brief Struct to define native functions
 */
typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

/**
 * @class ObjString
 * @brief Child struct for string type
 *
 */
struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

/**
 * @class ObjUpvalue
 * @brief Upvalue type struct.
 *
 */
typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

/**
 * @class ObjClosure
 * @brief Closure-type struct
 *
 */
typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

/**
 * @class ObjClosure
 * @brief Class-type struct
 *
 */
typedef struct {
    Obj obj;
    ObjString* name;
    Table methods;
} ObjClass;

/**
 * @class ObjInstance
 * @brief Struct to represent an obj instance
 *
 */
typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

/**
 * @class ObjBoundMethod
 * @brief Struct to represent a method bound to an object
 *
 */
typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

/**
 * @brief Method to create a new bound method
 *
 * @param receiver The receiver of the binding. Set to value to avoid 
 * casting it to the correct value every time.
 * @param method The method implementation as a closure
 * @return 
 */
ObjBoundMethod* newBoundMethod(VM* vm, Value receiver, ObjClosure* method);

/**
 * @brief Method to create an ObjClass.
 *
 * @param name The name of the class.
 * @return ObjClass* Pointer to a the new ObjClass struct.
 */
ObjClass* newClass(VM* vm, ObjString* name);

/**
 * @brief Method to create an ObjClosure.
 *
 * @param function The function object to hold.
 * @return ObjClosure* Pointer to an Objclosure struct.
 */
ObjClosure* newClosure(VM* vm, ObjFunction* function);

/**
 * @brief Method to create an ObjFunction.
 *
 * @return ObjFunction* A pointer to a new function object.
 */
ObjFunction* newFunction(VM* vm, FunctionType type);

/**
 * @brief Method to create an ObjInstance
 *
 * @param klass The class that the instance is a part of
 * @return 
 */
ObjInstance* newInstance(VM* vm, ObjClass* klass);

/**
 * @brief Method to create a new native function
 *
 * @param function A pointer to a native function
 * @return ObjNative* A pointer to the native function created
 */
ObjNative* newNative(VM* vm, NativeFn function);

/**
 * @brief Method to create an ObjString given a C-string. Assumes ownership
 * of the new string created.
 *
 * @param chars The C-string representation
 * @param length The number of characters in the string
 * @return ObjString* A pointer to an ObjString
 */
ObjString* takeString(VM* vm, char* chars, int length);

/**
 * @brief Method to copy the C-string into an ObjString. Assumes no ownership
 * of the characters passed in as args.
 *
 * @param chars The C-string representation
 * @param length The number of characters in the string
 * @return ObjString* A pointer to an ObjString
 */
ObjString* copyString(VM* vm, const char* chars, int length);

/**
 * @brief Method to create a new upvalue object
 *
 * @param slot The slot where the upvalue is slotted
 * @return ObjUpvalue* A pointer to a ObjUpvalue
 */
ObjUpvalue* newUpvalue(VM* vm, Value* slot);

/**
 * @brief Method to print the values of the object
 *
 * @param value Value to print
 */
void printObject(Value value);

/**
 * @brief Method to check if a given value is of a specified object type
 *
 * @param value Value for which we will check its type
 * @param type The object type being compared
 * @return true if value is of type ObjType
 */
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
