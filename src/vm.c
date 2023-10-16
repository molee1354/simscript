#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// check for platform
#ifdef _WIN64
#include <dos.h>
#else
#include <unistd.h>
#endif

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

/**
 * @brief Global vm instance to be referred to by all the methods. 
 * May later be an argument to each of the functions.
 *
 */
VM vm;

// Runtime error function declaration
static void runtimeError(const char* format, ...);

/**
 * @brief Defining the native "clock()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value The elapsed time since the program started running
 */
static Value clockNative(int argCount __attribute__((unused)),
        Value* args __attribute__((unused))) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

/**
 * @brief Defining the native "sleep()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value Null val.
 */
static Value sleepNative(int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError("Too many arguments provided : %d", argCount);
        return NULL_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("Incorrect argument type.");
        return NULL_VAL;
    }
    int waitFor = (int)AS_NUMBER(args[0]);

    /* clock_t timeStart = clock();
    while (clock() < timeStart + waitFor)
        ; */
    sleep(waitFor);
    return NULL_VAL;
}

/**
 * @brief Method to exit the code with an exitcode
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value No return value
 */
static Value exitNative(int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError("Too many arguments provided : %d", argCount);
        return NULL_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("Incorrect argument type.");
        return NULL_VAL;
    }
    int exitCode = (int)AS_NUMBER(args[0]);
    printf("Program exit with exitcode %d\n", exitCode);
    exit(exitCode);
}

/**
 * @brief Defining the native "puts()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value NULL_VAL
 */
static Value putsNative(int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError("Too many arguments provided : %d", argCount);
        return NULL_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError("Incorrect argument type.");
        return NULL_VAL;
    }
    printf("%s\n", AS_CSTRING(args[0]));
    return NULL_VAL;
}

/**
 * @brief Defining the native "system()" function.
 *
 * @param argcount The number of arguments 
 * @param args The arguments
 * @return Value NULL_VAL
 */
static Value systemNative(int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError("Too many arguments provided : %d", argCount);
        return NULL_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError("Incorrect argument type.");
        return NULL_VAL;
    }
    system(AS_CSTRING(args[0]));
    return NULL_VAL;
}

/**
 * @brief Method to reset the VM stack
 */
static void resetStack() {
    // setting the stackTop pointer to the beginning of the stack
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

/**
 * @brief Method to handle runtime errors
 * @param format The print format for error messaging
 *
 */
static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    /* Running through the stacktrace to track runtime errors
     * set vm.frameCount-1 since we want the stack trace to point to the 
     * previous failed instruction
     */
    for (int i = vm.frameCount-1; i>=0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else{
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack();
}

/**
 * @brief Method to define new native functions. It takes a pointer to a C
 * function and the name it will be known in the language implementation
 *
 * @param name Name of native function
 * @param function Pointer to C function
 */
static void defineNative(const char* name, NativeFn function) {
    push( OBJ_VAL(copyString(name, (int)strlen(name))) );
    push( OBJ_VAL(newNative(function)) );
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024; // arbitrary initial starting threshold

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);

    // defining native functions
    defineNative("clock", clockNative);
    defineNative("puts", putsNative);
    defineNative("exit", exitNative);
    defineNative("sleep", sleepNative);
    defineNative("system", systemNative);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    vm.initString = NULL;
    freeObjects();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

/**
 * @brief Method to call a function object
 *
 * @param function Pointer to the function to call
 * @param argCount The number of arguments to the function
 * @return bool True
 */
static bool call(ObjClosure* closure, int argCount) {
    // checking argument numbers
    if (argCount != closure->function->params) {
        runtimeError("Expected %d arguments but got %d.",
                closure->function->params, argCount);
        return false;
    }
    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }
    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;

    // ensuring that the arguments on the stack line up with func params
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

/**
 * @brief Method to execute the call to a callable object
 *
 * @param callee Object to call
 * @param argCount The number of arguments
 * @return bool True if object is callable.
 */
static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount-1] = bound->receiver;
                return call(bound->method, argCount);
            }
            case OBJ_CLASS: { // calling a new class to create an instance
                ObjClass* klass = AS_CLASS(callee);
                vm.stackTop[-argCount-1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE:
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop-argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            default:
                break; // non-callable object type
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

/**
 * @brief Function to invoke the correct method from a class.
 *
 * @param klass The class from which to find the method from
 * @param name The name of the called method
 * @param argCount The number of arguments
 * @return bool True if invocation from given class was successful
 */
static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined method '%s' in class '%s'.",
                name->chars, klass->name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}

/**
 * @brief Method to determine if the invocation is successful.
 *
 * @param name Name of possibly invoked method
 * @param argCount Number of arguments passed into method call
 * @return bool True if invocation is successful
 */
static bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);
    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods. Method '%s' not found.",
                     name->chars);
        return false;
    }
    ObjInstance* instance = AS_INSTANCE(receiver);

    // before we look up a method in a class, we look for a field with the
    // same name.
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount -1] = value;
        return callValue(value, argCount);
    }
    return invokeFromClass(instance->klass, name, argCount);
}

/**
 * @brief Method to bind a method to a class
 *
 * @param klass The class to bind to 
 * @param name The name of the method
 * @return bool True if method is defined and bound
 */
static bool bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

/**
 * @brief Method to capture a new upvalue
 *
 * @param local The value to capture
 * @return ObjUpvalue* A pointer to the caputred upvalue
 */
static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    // traversing the linked list of upvalues
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue; // return an existing upvalue
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue; // add to head if empty head 
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
static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL &&
           vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

/**
 * @brief Method to define a class method
 *
 * @param name Name of method
 */
static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
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
static void toString(Value value) {
    if (!IS_NUMBER(value)) {
        runtimeError("Unsupported type conversion to string.");
    }
    double num = (double)AS_NUMBER(value);
    int length;
    length = snprintf(NULL, 0, "%g", num);
    char* numStr = toChar(num, length+1);
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, numStr, length);

    ObjString* conversion = takeString(chars, length);
    pop();
    push(OBJ_VAL(conversion));
    free(numStr);
}

static void concatenate() {
    
    if (!IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        toString(peek(0));
    } else if (IS_STRING(peek(0)) && !IS_STRING(peek(1))) {
        Value temp = peek(0);
        pop();
        toString(peek(0));
        push(temp);
    }

    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    // the total length of the new string
    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length); // ptr offset by a->length
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount-1];

// ip set to the instruction about to be executed
#define READ_BYTE()     ( *frame->ip++ ) 

#define READ_SHORT() \
    ( frame->ip += 2, \
      (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]) )

#define READ_CONSTANT() \
    ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

#define READ_STRING()   AS_STRING(READ_CONSTANT())

// macro for binary operation handling
#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // printing the stack 
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        // pass in the current callframe instead of the vm's chunk and ip fields
        disassembleInstruction( &frame->closure->function->chunk,
                (int)(frame->ip - frame->closure->function->chunk.code) );
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NULL:  push(NULL_VAL); break;
            case OP_TRUE:  push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP:   pop(); break;
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();    
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Only instances have callable properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;
                // if the instance has the field with the name
                if (tableGet(&instance->fields, name, &value)) {
                    pop();
                    push(value);
                    break;
                }
                if(!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                // ????????
                // runtimeError("Undefined property '%s' in '%s'.",
                //         name->chars,
                //         instance->klass->name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());

                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL( valuesEqual(a, b)) );
                break;
            }

            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;

            case OP_ADD: {
                if ( IS_STRING(peek(0)) || IS_STRING(peek(1)) ) {
                    concatenate();
                } else if( IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)) ) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a+b));
                } else {
                    runtimeError(
                            "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_MOD: {
                if ( (IS_NUMBER(peek(0)) && IS_NUMBER(peek(0))) &&
                     (AS_NUMBER(peek(0)) == (int)AS_NUMBER(peek(0))) &&
                     (AS_NUMBER(peek(1)) == (int)AS_NUMBER(peek(1))) 
                   ) {
                    
                    int b = (int)AS_NUMBER(pop());
                    int a = (int)AS_NUMBER(pop());
                    push(NUMBER_VAL(a%b));
                } else {
                    runtimeError(
                            "Operands must be two integers.");
                    return INTERPRET_RUNTIME_ERROR;;
                }
                break;
            }

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push( NUMBER_VAL( -AS_NUMBER(pop()) ) );
                break;
            }
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_IMPORT: {
                printValue(pop());
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
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount-1];
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if(!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount-1];
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount-1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i=0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = 
                            captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_RETURN: {
                // holding onto the return value of the function
                Value result = pop();
                closeUpvalues(frame->slots);
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }
                vm.stackTop = frame->slots;
                push(result); // pushing the return value back onto the stack
                frame = &vm.frames[vm.frameCount-1];
                break;
            }
            case OP_CLASS:
                push( OBJ_VAL(newClass(READ_STRING())) );
                break;
            case OP_INHERIT: {
                Value superclass = peek(1); // superclass def top 
                                            //[<subclass>, <superclass>]
                if (!IS_CLASS(superclass)) {
                    runtimeError("Cannot inherit from non-class object.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods,
                            &subclass->methods);
                pop();
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);

    return run();
}
