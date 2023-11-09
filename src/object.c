#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

/**
 * @brief Macro wrapper to allocateObject wrapper. Casts output to type
 *
 */
#define ALLOCATE_OBJ(vm, type, objectType) \
    (type*)allocateObject(vm, sizeof(type), objectType)

/**
 * @brief Method to allocate memory for an Obj struct on the heap
 *
 * @param size Size of memory allocation
 * @param type ObjType type of Obj
 * @return Obj* Pointer to an Obj struct
 */
static Obj* allocateObject(VM* vm, size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(vm, NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    // inserting the allocated object into the list
    object->next = vm->objects; // setting next of new head to old head
    vm->objects = object; // setting new head to new object
#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif
    return object;
}

ObjModule* newModule(VM* vm, ObjString* name) {
    Value moduleVal;
    if (tableGet(&vm->modules, name, &moduleVal)) {
        return AS_MODULE(moduleVal);
    }

    ObjModule* module = ALLOCATE_OBJ(vm, ObjModule, OBJ_MODULE);
    initTable(&module->values);
    module->name = name;
    module->path = NULL;

    push(vm, OBJ_VAL(module));
    ObjString* __file__ = copyString(vm, "__file__", 8);
    push(vm, OBJ_VAL(__file__));

    tableSet(vm, &module->values, __file__, OBJ_VAL(name));
    tableSet(vm, &vm->modules, name, OBJ_VAL(module));

    pop(vm);
    pop(vm);

    return module;
}

ObjList* newList(VM* vm) {
    ObjList* list = ALLOCATE_OBJ(vm, ObjList, OBJ_LIST);
    initValueArray(&list->items);
    return list;
}

void appendList(VM *vm, ObjList *list, Value value) {
    writeValueArray(vm, &list->items, value);
}

bool validIndexList(VM *vm, ObjList *list, int index) {
    UNUSED(vm);
    if (index < 0)
        index += list->items.count; // negative indices start from back
    if (index < list->items.count && index >= 0)
        return true;
    return false;
}

Value getFromIndexList(VM *vm, ObjList *list, int index) {
    UNUSED(vm);
    if (index < 0)
        index += list->items.count;
    return list->items.values[index];
}

void setToIndexList(VM *vm, ObjList *list, int index, Value value) {
    UNUSED(vm);
    if (index<0)
        index += list->items.count;
    list->items.values[index] = value;
}

void deleteFromIndexList(VM *vm, ObjList *list, int index) {
    UNUSED(vm);
    if (index<0)
        index += list->items.count;
    for (int i = index; i < list->items.count-1; i++) {
        list->items.values[i] = list->items.values[i+1];
    }
    list->items.count--;
}

void clearList(VM* vm, ObjList* list) {
    UNUSED(vm);
    for (int i = 0; i<list->items.count; i++) {
        deleteFromIndexList(vm, list, i);
    }
}

ObjBoundMethod* newBoundMethod(VM* vm, Value receiver, ObjClosure* method) {
    ObjBoundMethod* bound = ALLOCATE_OBJ(vm, ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass* newClass(VM* vm, ObjString* name) {
    ObjClass* klass = ALLOCATE_OBJ(vm, ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjClosure* newClosure(VM* vm, ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(vm, ObjUpvalue*, function->upvalueCount);
    for (int i=0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(vm, ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction* newFunction(VM* vm, ObjModule* module, FunctionType type) {
    ObjFunction* function = ALLOCATE_OBJ(vm, ObjFunction, OBJ_FUNCTION);
    function->params = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    function->type = type;
    function->module = module;
    initChunk(vm, &function->chunk);
    return function;
}

ObjInstance* newInstance(VM* vm, ObjClass* klass) {
    ObjInstance* instance = ALLOCATE_OBJ(vm, ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjNative* newNative(VM* vm, NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(vm, ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

/**
 * @brief Method to allocate an ObjString
 *
 * @param chars Character array that represents the string
 * @param length Length of character array
 * @param hash Hash code for the string
 * @return ObjString* Pointer to the ObjString object
 */
static ObjString* allocateString(VM* vm, char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(vm, OBJ_VAL(string));

    // we only care about the keys, so values are NULL
    tableSet(vm, &vm->strings, string, NULL_VAL);
    pop(vm);
    return string;
}

/**
 * @brief Hash function for language implementation
 *
 * @param key the key to hash 
 * @param length the length of the key
 * @return uint32_t Hash code for string
 */
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(VM* vm, char* chars, int length) {
    uint32_t hash = hashString(chars, length); // calculate hash code for string
                                               //
    // look for string in string table
    ObjString* interned = tableFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) {
        // if found, we free before we return it since this function
        // is the owner and we don't need the string
        FREE_ARRAY(vm, char, chars, length+1);
        return interned;
    }
    return allocateString(vm, chars, length, hash);
}

ObjString* copyString(VM* vm, const char* chars, int length) {
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(vm, char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(vm, heapChars, length, hash);
}

ObjUpvalue* newUpvalue(VM* vm, Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(vm, ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NULL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}
/**
 * @brief Method to print a function
 *
 * @param function Pointer to a function object.
 */
static void printFunction(FILE* file, ObjFunction* function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    fprintf(file, "<fn %s>", function->name->chars);
}

void printObject(FILE* file, Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_MODULE:
            fprintf(file, "%s", AS_MODULE(value)->name->chars);
            break;
        case OBJ_LIST: {
            ObjList* list = AS_LIST(value);
            fprintf(file, "[");
            for (int i=0; i<list->items.count; i++) {
                printValue(file, list->items.values[i]);
                if (i != list->items.count-1) fprintf(file, ", ");
            }
            fprintf(file, "]");
            break;
        }
        case OBJ_BOUND_METHOD:
            printFunction(file, AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJ_CLASS:
            fprintf(file, "%s", AS_CLASS(value)->name->chars);
            break;
        case OBJ_CLOSURE:
            printFunction(file, AS_CLOSURE(value)->function);
            break;
        case OBJ_FUNCTION:
            printFunction(file, AS_FUNCTION(value));
            break;
        case OBJ_NATIVE:
            fprintf(file, "<native function>");
            break;
        case OBJ_STRING:
            fprintf(file, "%s", AS_CSTRING(value));
            break;
        case OBJ_UPVALUE:
            fprintf(file, "upvalue");
            break;
        case OBJ_INSTANCE:
            fprintf(file, "%s instance",
                    AS_INSTANCE(value)->klass->name->chars);
            break;
    }
}

