#include "../natives.h"
#include "maths.h"
#include <math.h>


static Value sineMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.sin(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(sin(AS_NUMBER((double)args[0])));
}

static Value cosineMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.cos(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(cos(AS_NUMBER((double)args[0])));
}

static Value tangentMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.tan(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(tan(AS_NUMBER((double)args[0])));
}

static Value arcsinMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.asin(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(asin(AS_NUMBER((double)args[0])));
}

static Value arccosMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.acos(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(acos(AS_NUMBER((double)args[0])));
}

static Value arctanMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.atan(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(atan(AS_NUMBER((double)args[0])));
}

static Value floorMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.floor(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(floor(AS_NUMBER((double)args[0])));
}

static Value ceilMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.ceil(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(ceil(AS_NUMBER((double)args[0])));
}

static Value logEMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.ln(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(log(AS_NUMBER((double)args[0])));
}

static Value log10Math(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.log(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(log10(AS_NUMBER((double)args[0])));
}

static Value sqrtMath(VM* vm, int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError(vm, "'Math.sqrt(arg)' takes exactly least one argument (%d provided)",
                     argCount);
        return BAD_VAL;
    }
    return NUMBER_VAL(sqrt(AS_NUMBER((double)args[0])));
}

ObjModule* initLib_Math(VM* vm) {
    ObjString* name = copyString(vm, "Error", 5);
    push(vm, OBJ_VAL(name));
    ObjModule* mathLib = newModule(vm, name);
    push(vm, OBJ_VAL(mathLib));
    defineNative(vm, &mathLib->values, "sin", sineMath);
    defineNative(vm, &mathLib->values, "cos", cosineMath);
    defineNative(vm, &mathLib->values, "tan", tangentMath);
    defineNative(vm, &mathLib->values, "asin", arcsinMath);
    defineNative(vm, &mathLib->values, "acos", arccosMath);
    defineNative(vm, &mathLib->values, "atan", arctanMath);
    defineNative(vm, &mathLib->values, "ceil", ceilMath);
    defineNative(vm, &mathLib->values, "floor", floorMath);
    defineNative(vm, &mathLib->values, "ln", logEMath);
    defineNative(vm, &mathLib->values, "log", log10Math);
    defineNative(vm, &mathLib->values, "sqrt", sqrtMath);
    pop(vm);
    pop(vm);
    return mathLib;
}
