#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(VM* vm, void* pointer, size_t oldSize, size_t newSize) {
    // calling reallocate for more memory runs a garbage collection
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        // collect according to a threshold value
        if (vm->bytesAllocated > vm->nextGC) {
            collectGarbage(vm);
        }
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

void markObject(VM* vm, Obj *object) {
    if (object == NULL) return;
    if (object->isMarked) return; // if object is marked, don't mark it
#ifdef DEBUG_LOG_GC
    printf("%p mark", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif
    object->isMarked = true;

    if (vm->grayCapacity < vm->grayCount + 1) {
        vm->grayCapacity = GROW_CAPACITY(vm->grayCapacity);
        vm->grayStack = (Obj**)realloc(vm->grayStack,
                                      sizeof(Obj*) * vm->grayCapacity);
        if (vm->grayStack == NULL) exit(1);
    }
    vm->grayStack[vm->grayCount++] = object;
}

void markValue(VM* vm, Value value) {
    if (IS_OBJ(value)) markObject(vm, AS_OBJ(value));
}

/**
 * @brief Method to mark an array
 *
 * @param array Array to mark
 */
static void markArray(VM* vm, ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(vm, array->values[i]);
    }
}

/**
 * @brief Traversing an object's references to make them "black". A black
 * object is any object whose 'isMarked' field is set and is no longer in
 * the gray stack.
 *
 * @param object The current object that is being traversed.
 */
static void blackenObject(VM* vm, Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif
    switch (object->type) {
        case OBJ_MODULE: {
            ObjModule* module = (ObjModule*) object;
            markObject(vm, (Obj*)module->name);
            markObject(vm, (Obj*)module->path);
            markTable(vm, &module->values);
            break;
        }
        case OBJ_LIST: {
            ObjList* list = (ObjList*) object;
            markArray(vm, &list->items);
            break;
        }
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*)object;
            markValue(vm, bound->receiver);
            markObject(vm, (Obj*)bound->method);
            break;
        }
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)object;
            markObject(vm, (Obj*)klass->name);
            markTable(vm, &klass->methods);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            markObject(vm, (Obj*)closure->function);

            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject(vm, (Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            markObject(vm, (Obj*)function->name);
            markArray(vm, &function->chunk.constants);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            markObject(vm, (Obj*)instance->klass);
            markTable(vm, &instance->fields);
            break;
        }
        case OBJ_UPVALUE:
            markValue(vm, ((ObjUpvalue*)object)->closed);
            break;
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

/**
 * @brief Method to free an individual Obj* based on its type
 *
 * @param object Object to free
 */
static void freeObject(VM* vm, Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif
    switch (object->type) {
        case OBJ_MODULE: {
            ObjModule* module = (ObjModule*)object;
            freeTable(vm, &module->values);
            FREE(vm, ObjModule, object);
            break;
        }
        case OBJ_LIST: {
            ObjList* list = (ObjList*)object;
            freeValueArray(vm, &list->items);
            FREE(vm, ObjList, object);
            break;
        }
        case OBJ_BOUND_METHOD: {
            FREE(vm, ObjBoundMethod, object);
            break;
        }
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)object;
            freeTable(vm, &klass->methods);
            FREE(vm, ObjClass, object);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            FREE_ARRAY(vm, ObjUpvalue*, closure->upvalues,
                       closure->upvalueCount);
            FREE(vm, ObjClosure, object);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(vm, &function->chunk);
            FREE(vm, ObjFunction, object);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            freeTable(vm, &instance->fields);
            FREE(vm, ObjInstance, object);
            break;
        }
        case OBJ_NATIVE: {
            FREE(vm, OBJ_NATIVE, object); // native obj don't hold extra memory
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(vm, char, string->chars, string->length+1);
            FREE(vm, ObjString, object);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(vm, ObjUpvalue, object);
            break;
        }
    }
}

static void markRoots(VM* vm) {
    // marking values
    for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
        markValue(vm, *slot);
    }
    
    // marking closures
    for (int i = 0; i < vm->frameCount; i++) {
        markObject(vm, (Obj*)vm->frames[i].closure);
    }

    // marking upvalues
    for (ObjUpvalue* upvalue = vm->openUpvalues; upvalue != NULL;
            upvalue = upvalue->next) {
        markObject(vm, (Obj*)upvalue);
    }

    markTable(vm, &vm->globals);
    markTable(vm, &vm->modules);
    markTable(vm, &vm->listMethods);
    markTable(vm, &vm->stringMethods);
    markCompilerRoots(vm);
    markObject(vm, (Obj*)vm->initString);
}

/**
 * @brief Method to trace the references in the current vm
 *
 */
static void traceReferences(VM* vm) {
    while (vm->grayCount > 0) {
        Obj* object = vm->grayStack[--vm->grayCount];
        blackenObject(vm, object);
    }
}

static void sweep(VM* vm) {
    Obj* previous = NULL;
    Obj* object = vm->objects;

    while (object != NULL) {
        if (object->isMarked) { // if marked (black) don't touch
            object->isMarked = false; // unmark for next round of sweep
            previous = object;
            object = object->next;
        } else { // if not marked (white) unlink and free
            Obj* unreached = object;
            object = object->next;
            if(previous != NULL) {
                previous->next = object;
            } else {
                vm->objects = object;
            }

            freeObject(vm, unreached);
        }
    }
}

void collectGarbage(VM* vm) {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    markRoots(vm);
    traceReferences(vm);
    tableRemoveWhite(vm, &vm->strings); // removing dangling pointers to strings
    sweep(vm);

    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
            before - vm.bytesAllocated, before, vm.bytesAllocated,
            vm.nextGC);
#endif
}

void freeObjects(VM* vm) {
    Obj* object = vm->objects;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(vm, object);
        object = next;
    }
    free(vm->grayStack);
}
