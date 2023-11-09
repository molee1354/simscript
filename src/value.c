#include "object.h"
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(VM* vm, ValueArray *array, Value value) {
    // checking to see if array has enough capacity
    if (array->capacity < array->count+1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(vm, Value,
                array->values,
                oldCapacity,
                array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(VM* vm, ValueArray* array) {
    FREE_ARRAY(vm, uint8_t, array->values, array->capacity);
    initValueArray(array); // zero out the fields so it's in an empty state
}

void printValue(FILE *file, Value value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        fprintf(file, AS_BOOL(value) ? "true" : "false");
    } else if (IS_NULL(value)) {
        fprintf(file, "null");
    } else if (IS_NUMBER(value)) {
        fprintf(file, "%g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        printObject(file, value);
    }
#else
    switch (value.type) {
        case VAL_BOOL:
            fprintf(file, AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NULL: fprintf(file, "null"); break;
        case VAL_NUMBER: fprintf(file, "%g", AS_NUMBER(value)); break;
        case VAL_OBJ: printObject(file, value); break;
    }
#endif
}

static bool compareObj(Value a, Value b) {
    if (AS_OBJ(a)->type != AS_OBJ(b)->type)
        return false;
    switch (AS_OBJ(a)->type) {
        default:
            return AS_OBJ(a) == AS_OBJ(b);
        case OBJ_LIST: {
            ObjList* list1 = AS_LIST(a);
            ObjList* list2 = AS_LIST(b);
            if (list1->items.count != list2->items.count)
                return false;
            for (int i = 0; i<list1->items.count; i++) {
                if (list1->items.values[i] != list2->items.values[i])
                    return false;
            }
            return true;
        }
    }
}

bool valuesEqual(Value a, Value b) {
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    if (IS_OBJ(a) && IS_OBJ(b)) {
        return compareObj(a, b);
    }
    return a == b;
#else
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NULL:   return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        default:         return false; // unreachable
    }
#endif
}
