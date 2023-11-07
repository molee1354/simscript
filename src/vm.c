#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "SVM.h"
#include "library.h"
#include "table.h"
#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "natives.h"
#include "object.h"
#include "read.h"
#include "value.h"

#include "objs/list.h"

/**
 * @brief Global vm instance to be referred to by all the methods. 
 * May later be an argument to each of the functions.
 *
 */

/**
 * @brief Method to reset the VM stack
 */
static void resetStack(VM* vm) {
    // setting the stackTop pointer to the beginning of the stack
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
    vm->openUpvalues = NULL;
    vm->compiler = NULL;
}

/**
 * @brief Method to handle runtime errors
 * @param format The print format for error messaging
 *
 */
void runtimeError(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    fprintf(stderr, "\nRUNTIME ERROR:\n");
#else
    fprintf(stderr, "\n\033[0;31mRUNTIME ERROR:\033[0m\n");
#endif
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    /* Running through the stacktrace to track runtime errors
     * set vm.frameCount-1 since we want the stack trace to point to the 
     * previous failed instruction
     */
    for (int i = vm->frameCount-1; i>=0; i--) {
        CallFrame* frame = &vm->frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;

        fprintf(stderr, "  @ '%s', line %d in ",
                function->module->name->chars,
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else{
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack(vm);
}

void runtimeWarning(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    fprintf(stderr, "\nWARNING:");
#else
    fprintf(stderr, "\n\033[0;33mWARNING:\033[0m ");
#endif
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    for (int i = vm->frameCount-1; i>=0; i--) {
        CallFrame* frame = &vm->frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;

        fprintf(stderr, "  @ '%s', line %d in ",
                function->module->name->chars,
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else{
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
}

VM* initVM(bool repl) {
    VM* vm = (VM*)malloc(sizeof(VM));
    resetStack(vm);
    vm->repl = repl;
    vm->objects = NULL;

    vm->bytesAllocated = 0;
    vm->nextGC = 1024 * 1024; // arbitrary initial starting threshold

    vm->grayCount = 0;
    vm->grayCapacity = 0;
    vm->grayStack = NULL;

    vm->lastModule = NULL;

    initTable(&vm->globals);
    initTable(&vm->strings);
    initTable(&vm->modules);

    initTable(&vm->listMethods);

    vm->initString = NULL;
    vm->initString = copyString(vm, "init", 4);

    // defining native functions
    defineNatives(vm);
    defineListMethods(vm);
    return vm;
}

void freeVM(VM* vm) {
    freeTable(vm, &vm->globals);
    freeTable(vm, &vm->strings);
    freeTable(vm, &vm->modules);
    freeTable(vm, &vm->listMethods);
    vm->initString = NULL;
    freeObjects(vm);
    free(vm);
}

void push(VM* vm, Value value) {
    *vm->stackTop = value;
    vm->stackTop++;
}

Value pop(VM* vm) {
    vm->stackTop--;
    return *vm->stackTop;
}

static Value peek(VM* vm, int distance) {
    return vm->stackTop[-1 - distance];
}

/**
 * @brief Method to call a function object
 *
 * @param function Pointer to the function to call
 * @param argCount The number of arguments to the function
 * @return bool True
 */
static bool call(VM* vm, ObjClosure* closure, int argCount) {
    // checking argument numbers
    if ( argCount != closure->function->params ) {
        runtimeError(vm, "Function %s() expected %d arguments but got %d.",
                closure->function->name->chars,
                closure->function->params,
                argCount);
        return false;
    }
    if (vm->frameCount == FRAMES_MAX) {
        runtimeError(vm, "Stack overflow.");
        return false;
    }
    CallFrame* frame = &vm->frames[vm->frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    // ensuring that the arguments on the stack line up with func params
    frame->slots = vm->stackTop - argCount - 1;
    return true;
}

/**
 * @brief Method to execute the call to a callable object
 *
 * @param callee Object to call
 * @param argCount The number of arguments
 * @return bool True if object is callable.
 */
static bool callValue(VM* vm, Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm->stackTop[-argCount-1] = bound->receiver;
                return call(vm, bound->method, argCount);
            }
            case OBJ_CLASS: { // calling a new class to create an instance
                ObjClass* klass = AS_CLASS(callee);
                vm->stackTop[-argCount-1] = OBJ_VAL(newInstance(vm, klass));
                Value initializer;
                if (tableGet(&klass->methods, vm->initString, &initializer)) {
                    return call(vm, AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError(vm, "Expected 0 arguments but got %d.", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE:
                return call(vm, AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE:
                NativeFn native = AS_NATIVE(callee);
                Value result = native(vm, argCount, vm->stackTop-argCount);
                vm->stackTop -= argCount + 1;
                push(vm, result);
                return true;
            default:
                break; // non-callable object type
        }
    }
    runtimeError(vm, "Can only call functions and classes.");
    return false;
}

static bool callNativeMethod(VM* vm, Value method, int argCount) {
    NativeFn native = AS_NATIVE(method);
    Value result = native(vm, argCount, vm->stackTop-argCount-1);

    if (IS_BAD(result)) return false;

    vm->stackTop -= argCount +1;
    push(vm, result);
    return true;
}

/**
 * @brief Function to invoke the correct method from a class.
 *
 * @param klass The class from which to find the method from
 * @param name The name of the called method
 * @param argCount The number of arguments
 * @return bool True if invocation from given class was successful
 */
static bool invokeFromClass(VM* vm, ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined method '%s' in class '%s'.",
                name->chars, klass->name->chars);
        return false;
    }
    return call(vm, AS_CLOSURE(method), argCount);
}

/**
 * @brief Method to determine if the invocation is successful.
 *
 * @param name Name of possibly invoked method
 * @param argCount Number of arguments passed into method call
 * @return bool True if invocation is successful
 */
static bool invoke(VM* vm, ObjString* name, int argCount) {
    Value receiver = peek(vm, argCount);
    if (!IS_OBJ(receiver)) {
        runtimeError(vm, "Invalid method call '%s()' to unsupported type.",
                     name->chars);
        return false;
    }
    switch (getObjType(receiver)) {
        default:
            return false;
        case OBJ_MODULE: {
            ObjModule* module = AS_MODULE(receiver);
            Value value;
            if (!tableGet(&module->values, name, &value)) {
                runtimeError(vm, "Could not access field '%s' in module %s.",
                             name->chars, module->name->chars);
                return false;
            }
            return callValue(vm, value, argCount);
        }
        case OBJ_LIST: {
            Value value;
            if (!tableGet(&vm->listMethods, name, &value)) {
                runtimeError(vm, "No list method %s() found.", name->chars);
                return false;
            }
            return callNativeMethod(vm, value, argCount);
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(receiver);

            // before we look up a method in a class, we look for a field with the
            // same name.
            Value value;
            if (tableGet(&instance->fields, name, &value)) {
                vm->stackTop[-argCount -1] = value;
                return callValue(vm, value, argCount);
            }
            return invokeFromClass(vm, instance->klass, name, argCount);
        }
    }
}

/**
 * @brief Method to bind a method to a class
 *
 * @param klass The class to bind to 
 * @param name The name of the method
 * @return bool True if method is defined and bound
 */
static bool bindMethod(VM* vm, ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError(vm, "Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(vm, peek(vm, 0), AS_CLOSURE(method));
    pop(vm);
    push(vm, OBJ_VAL(bound));
    return true;
}

/**
 * @brief Method to capture a new upvalue
 *
 * @param local The value to capture
 * @return ObjUpvalue* A pointer to the caputred upvalue
 */
static ObjUpvalue* captureUpvalue(VM* vm, Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm->openUpvalues;
    // traversing the linked list of upvalues
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue; // return an existing upvalue
    }

    ObjUpvalue* createdUpvalue = newUpvalue(vm, local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm->openUpvalues = createdUpvalue; // add to head if empty head
    } else {
        prevUpvalue->next = createdUpvalue; // add to after prev if not empty
    }
    return createdUpvalue;
}

/**
 * @brief Method to close open upvalues
 *
 * @param last Pointer to the last upvalue visited
 */
static void closeUpvalues(VM* vm, Value* last) {
    while (vm->openUpvalues != NULL &&
           vm->openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm->openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm->openUpvalues = upvalue->next;
    }
}

/**
 * @brief Method to define a class method
 *
 * @param name Name of method
 */
static void defineMethod(VM* vm, ObjString* name) {
    Value method = peek(vm, 0);
    ObjClass* klass = AS_CLASS(peek(vm, 1));
    tableSet(vm, &klass->methods, name, method);
    pop(vm);
}

/**
 * @brief Method to determine if a value is "falsey"
 *
 * @param value Value to check false-ness
 * @return True if value is falsey
 */
static bool isFalsey(Value value) {
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/**
 * @brief Method to convert a number to a string.
 *
 * @param num Number to convert
 * @param length Number of characters in the number
 * @return char* Pointer to the head of the character array representation.
 */
static char* toChar(double num, int length) {
    char* str = (char*)malloc(length);
    if (str == NULL) {
        return NULL;
    }
    snprintf(str, length, "%g", num);
    return str;
}

/**
 * @brief Method to convert a number to a string. Pops the topmost value in
 * the stack, converts it to a string, and pushes it back.
 *
 * @param value Number value to convert to string
 */
static void toString(VM* vm, Value value) {
    if (!IS_NUMBER(value)) {
        runtimeError(vm, "Unsupported type conversion to string.");
        return;
    }
    double num = (double)AS_NUMBER(value);
    int length;
    length = snprintf(NULL, 0, "%g", num);
    char* numStr = toChar(num, length+1);
    char* chars = ALLOCATE(vm, char, length+1);
    memcpy(chars, numStr, length);

    ObjString* conversion = takeString(vm, chars, length);
    pop(vm);
    push(vm, OBJ_VAL(conversion));
    free(numStr);
}

static void concatenate(VM* vm) {
    if (!IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
        toString(vm, peek(vm, 0));
    } else if (IS_STRING(peek(vm, 0)) && !IS_STRING(peek(vm, 1))) {
        Value temp = peek(vm, 0);
        pop(vm);
        toString(vm, peek(vm, 0));
        push(vm, temp);
    }

    ObjString* b = AS_STRING(peek(vm, 0));
    ObjString* a = AS_STRING(peek(vm, 1));
    if (a == NULL || b == NULL)  {
        runtimeError(vm, "Failed string conversion.");
        return;
    }

    // the total length of the new string
    int length = a->length + b->length;
    char* chars = ALLOCATE(vm, char, length+1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length); // ptr offset by a->length
    chars[length] = '\0';

    ObjString* result = takeString(vm, chars, length);
    pop(vm);
    pop(vm);
    push(vm, OBJ_VAL(result));
}

static InterpretResult run(VM* vm) {
    CallFrame* frame = &vm->frames[vm->frameCount-1];
    // register uint8_t* ip = frame->ip;

// ip set to the instruction about to be executed
#define READ_BYTE()     ( *frame->ip++ )

#define READ_SHORT() \
    ( frame->ip += 2, \
      (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]) )

#define READ_CONSTANT() \
    ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

#define READ_STRING()   AS_STRING(READ_CONSTANT())

// macro for binary operation handling
#define BINARY_OP(vm, valueType, op) \
    do { \
      if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
        runtimeError(vm, "Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop(vm)); \
      double a = AS_NUMBER(pop(vm)); \
      push(vm, valueType(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // printing the stack 
        printf("          ");
        for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
            printf("[ ");
            printValue(stdout, *slot);
            printf(" ]");
        }
        printf("\n");
        // pass in the current callframe instead of the vm->s chunk and ip fields
        disassembleInstruction( &frame->closure->function->chunk,
                (int)(frame->ip - frame->closure->function->chunk.code) );
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }

            case OP_NULL:  push(vm, NULL_VAL); break;
            case OP_TRUE:  push(vm, BOOL_VAL(true)); break;
            case OP_FALSE: push(vm, BOOL_VAL(false)); break;
            case OP_POP:   pop(vm); break;
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(vm,0);
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm, frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm->globals, name, &value)) {
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();    
                tableSet(vm, &vm->globals, name, peek(vm,0));
                pop(vm);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(vm, &vm->globals, name, peek(vm,0))) {
                    tableDelete(vm, &vm->globals, name);
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_MODULE: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&frame->closure->function->module->values, name, &value)) {
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, value);
                break;
            }
            case OP_DEFINE_MODULE: {
                ObjString* name = READ_STRING();    
                tableSet(vm, &frame->closure->function->module->values, name, peek(vm,0));
                pop(vm);
                break;
            }

            case OP_SET_MODULE: {
                ObjString* name = READ_STRING();
                if (tableSet(vm, &frame->closure->function->module->values, name, peek(vm,0))) {
                    tableDelete(vm, &frame->closure->function->module->values, name);
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_MAKE_LIST: {
                ObjList* list = newList(vm);
                uint8_t numElem = READ_BYTE();

                push(vm, OBJ_VAL(list));
                for (int i = numElem; i > 0; i--) {
                    appendList(vm, list, peek(vm, i));
                }
                vm->stackTop -= numElem+1;
                push(vm, OBJ_VAL(list));
                break;
            }
            case OP_SUBSCRIPT_ASSIGN: {
                Value item = pop(vm);
                Value possibleIndex = pop(vm);
                Value receiver = pop(vm);
                if (!IS_LIST(receiver)) {
                    runtimeError(vm, "Invalid subscript operation to unsupported type.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!IS_NUMBER(possibleIndex)) {
                    runtimeError(vm, "Subscript index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjList* list = AS_LIST(receiver);
                int index = AS_NUMBER(possibleIndex);

                if (!validIndexList(vm, list, index)) {
                    if (index > list->items.count) {
                        // "autovivification" with nulls
                        runtimeWarning(vm, "Index value greater than list capacity.");
                        for (int i = list->items.count; i < index-1; i++) {
                            appendList(vm, list, NULL_VAL);
                        }
                        appendList(vm, list, item);
                    } else {
                    runtimeError(vm, "List index out of bounds (given %d, length %d)",
                                index, list->items.count-1);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                setToIndexList(vm, list, index, item);
                push(vm, item);
                break;
            }
            case OP_SUBSCRIPT_IDX: {
                Value possibleIndex = pop(vm);
                Value receiver = pop(vm);
                Value value;

                if (!IS_OBJ(receiver)) {
                    runtimeError(vm, "Invalid subscript operation to unsupported type.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!IS_NUMBER(possibleIndex)) {
                    runtimeError(vm, "Subscript index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int index = AS_NUMBER(possibleIndex);
                switch (OBJ_TYPE(receiver)) {
                    default:
                        runtimeError(vm, "Invalid subscript operation to unsupported type.");
                        return INTERPRET_RUNTIME_ERROR;
                    case OBJ_LIST: {
                        ObjList* list = AS_LIST(receiver);
                        if (!validIndexList(vm, list, index)) {
                            runtimeError(vm, "List index out of bounds (given %d, length %d)",
                                    index, list->items.count-1);
                            return INTERPRET_RUNTIME_ERROR;
                        }
                        value = getFromIndexList(vm, list, index);
                        push(vm, value);
                        break;
                    }
                }
                break;
            }
            case OP_SUBSCRIPT_IDX_NOPOP: {
                Value possibleIndex = peek(vm, 0);
                Value receiver = peek(vm, 1);
                Value value;
                
                if (!IS_LIST(receiver)) {
                    runtimeError(vm, "Invalid subscript operation to unsupported type.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!IS_NUMBER(possibleIndex)) {
                    runtimeError(vm, "Subscript index must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int index = AS_NUMBER(possibleIndex);
                ObjList* list = AS_LIST(receiver);
                if (!validIndexList(vm, list, index)) {
                    runtimeError(vm, "List index out of bounds (given %d, length %d)",
                            index, list->items.count-1);
                    return INTERPRET_RUNTIME_ERROR;
                }
                value = getFromIndexList(vm, list, index);
                push(vm, value);
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(vm, *frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(vm,0);
                break;
            }
            case OP_GET_PROPERTY: {
                Value receiver = peek(vm, 0);

                if (isObjType(receiver, OBJ_INSTANCE)) {

                    ObjInstance* instance = AS_INSTANCE(peek(vm,0));
                    ObjString* name = READ_STRING();

                    Value value;
                    // if the instance has the field with the name
                    if (tableGet(&instance->fields, name, &value)) {
                        pop(vm);
                        push(vm, value);
                        break;
                    }
                    if(!bindMethod(vm, instance->klass, name)) {
                        return INTERPRET_RUNTIME_ERROR;
                    }
                } else if (isObjType(receiver, OBJ_MODULE)) {
                    ObjModule* module = AS_MODULE(receiver);
                    ObjString* name = READ_STRING();
                    Value value;
                    if (tableGet(&module->values, name, &value)) {
                        pop(vm);
                        push(vm, value);
                        break;
                    }
                    runtimeError(vm, "Module '%s' has no attribute '%s'.",
                            module->name->chars, name->chars);
                }
                return INTERPRET_RUNTIME_ERROR;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(vm,1))) {
                    runtimeError(vm, "Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(vm,1));
                tableSet(vm, &instance->fields, READ_STRING(), peek(vm,0));
                /* Value value = pop(vm);
                pop(vm);
                push(vm, value); */
                pop(vm);
                pop(vm);
                push(vm, NULL_VAL);
                break;
            }
            case OP_GET_PROPERTY_NOPOP: {
                
                if (!IS_INSTANCE(peek(vm,1))) {
                    runtimeError(vm, "Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(vm,1));
                ObjString* name = READ_STRING();
                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    push(vm, value);
                    break;
                }
                if (bindMethod(vm, instance->klass, name)) {
                    break;
                }

                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop(vm));

                if (!bindMethod(vm, superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, BOOL_VAL( valuesEqual(a, b)) );
                break;
            }

            case OP_GREATER:  BINARY_OP(vm, BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(vm, BOOL_VAL, <); break;

            case OP_ADD: {
                if ( IS_STRING(peek(vm,0)) || IS_STRING(peek(vm,1)) ) {
                    concatenate(vm);
                } else if( IS_NUMBER(peek(vm,0)) && IS_NUMBER(peek(vm,1)) ) {
                    double b = AS_NUMBER(pop(vm));
                    double a = AS_NUMBER(pop(vm));
                    push(vm, NUMBER_VAL(a+b));
                } else {
                    runtimeError(vm,
                            "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(vm, NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(vm, NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(vm, NUMBER_VAL, /); break;
            case OP_MOD: {
                if ( (IS_NUMBER(peek(vm,0)) && IS_NUMBER(peek(vm,0))) &&
                     (AS_NUMBER(peek(vm,0)) == (int)AS_NUMBER(peek(vm,0))) &&
                     (AS_NUMBER(peek(vm,1)) == (int)AS_NUMBER(peek(vm,1)))
                   ) {
                    
                    int b = (int)AS_NUMBER(pop(vm));
                    int a = (int)AS_NUMBER(pop(vm));
                    push(vm, NUMBER_VAL(a%b));
                } else {
                    runtimeError(vm, "Operands must be two integers.");
                    return INTERPRET_RUNTIME_ERROR;;
                }
                break;
            }
            case OP_INCREMENT: {
                if (!IS_NUMBER(peek(vm,0))) {
                    runtimeError(vm, "Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm,  NUMBER_VAL( AS_NUMBER(pop(vm))+1 ) );
                break;
            }
            case OP_DECREMENT: {
                if (!IS_NUMBER(peek(vm,0))) {
                    runtimeError(vm, "Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm,  NUMBER_VAL( AS_NUMBER(pop(vm))-1 ) );
                break;
            }
            case OP_MODULE: {
                ObjString* fileName = READ_STRING();
                Value moduleVal;

                char path[PATHLEN];
                if (!validPath(frame->closure->function->module->path->chars,
                            fileName->chars, path)) {
                    runtimeError(vm, "Could not open file '%s'.", fileName->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjString* pathObj = copyString(vm, path, strlen(path));
                push(vm, OBJ_VAL(pathObj));

                // skipping if file already imported
                if (tableGet(&vm->modules, pathObj, &moduleVal)) {
                    pop(vm);
                    vm->lastModule = AS_MODULE(moduleVal);
                    push(vm, NULL_VAL);
                    break;
                }

                char* source = readFile_VM(vm, path);

                if (source == NULL) {
                    runtimeError(vm, "Could not open file '%s'.", fileName->chars);
                }

                ObjModule* module = newModule(vm, pathObj);
                module->path = dirName(vm, path, strlen(path));
                vm->lastModule = module;

                pop(vm);
                push(vm, OBJ_VAL(module));
                ObjFunction* function = compile(vm, module, source);
                pop(vm);
                
                FREE_ARRAY(vm, char, source, strlen(source)+1);

                if (function == NULL) return INTERPRET_COMPILE_ERROR;
                push(vm, OBJ_VAL(function));
                ObjClosure* closure = newClosure(vm, function);
                pop(vm);
                push(vm, OBJ_VAL(closure));

//                frame->ip = ip;
                call(vm, closure, 0);
                frame = &vm->frames[vm->frameCount - 1];
//                ip = frame->ip;
                break;
            }
            case OP_MODULE_VAR: {
                push(vm, OBJ_VAL(vm->lastModule));
                break;
            }
            case OP_MODULE_END: {
                vm->lastModule = frame->closure->function->module;
                break;
            }
            case OP_MODULE_BUILTIN: {
                int index = READ_BYTE();
                ObjString* name = READ_STRING();
                Value stdLibVal;
                if (tableGet(&vm->modules, name, &stdLibVal)) {
                    push(vm, stdLibVal);
                    break;
                }
                ObjModule* stdLib = importStdLib(vm, index);
                push(vm, OBJ_VAL(stdLib));
                break;
            }
            case OP_NOT:
                push(vm, BOOL_VAL(isFalsey(pop(vm))));
                break;

            case OP_NEGATE: {
                if (!IS_NUMBER(peek(vm,0))) {
                    runtimeError(vm, "Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm,  NUMBER_VAL( -AS_NUMBER(pop(vm)) ) );
                break;
            }
            case OP_PRINT: {
                printValue(stdout, pop(vm));
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(vm,0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(vm, peek(vm,argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm->frames[vm->frameCount-1];
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if(!invoke(vm, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm->frames[vm->frameCount-1];
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop(vm));
                if (!invokeFromClass(vm, superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm->frames[vm->frameCount-1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(vm, function);
                push(vm, OBJ_VAL(closure));
                for (int i=0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = 
                            captureUpvalue(vm, frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm, vm->stackTop - 1);
                pop(vm);
                break;
            case OP_RETURN: {
                // holding onto the return value of the function
                Value result = pop(vm);
                closeUpvalues(vm, frame->slots);
                vm->frameCount--;
                if (vm->frameCount == 0) {
                    pop(vm);
                    return INTERPRET_OK;
                }
                vm->stackTop = frame->slots;
                push(vm, result); // pushing the return value back onto the stack
                frame = &vm->frames[vm->frameCount-1];
                break;
            }
            case OP_CLASS:
                push(vm,  OBJ_VAL(newClass(vm, READ_STRING())) );
                break;
            case OP_INHERIT: {
                Value superclass = peek(vm,1); // superclass def top
                                            //[<subclass>, <superclass>]
                if (!IS_CLASS(superclass)) {
                    runtimeError(vm, "Cannot inherit from non-class object.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(vm,0));
                tableAddAll(vm, &AS_CLASS(superclass)->methods,
                            &subclass->methods);
                pop(vm);
                break;
            }
            case OP_METHOD:
                defineMethod(vm, READ_STRING());
                break;
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult interpret(VM* vm, char* moduleName, const char* source) {
    ObjString* name = copyString(vm, moduleName, strlen(moduleName));
    push(vm, OBJ_VAL(name));
    ObjModule* module = newModule(vm, name);
    pop(vm);

    push(vm, OBJ_VAL(module));
    module->path = getDirectory(vm, moduleName);
    pop(vm);

    ObjFunction* function = compile(vm, module, source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(vm, OBJ_VAL(function));
    ObjClosure* closure = newClosure(vm, function);
    pop(vm);
    push(vm, OBJ_VAL(closure));
    call(vm, closure, 0);

    InterpretResult result = run(vm);
    return result;
}
