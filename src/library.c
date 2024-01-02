#include <string.h>

#include "library.h"
#include "libs/error.h"
#include "libs/io.h"
#include "libs/maths.h"

StdLib libraries[] = {
    {"Error", &initLib_Error},
    {"IO", &initLib_IO},
    {"Math", &initLib_Math},
    {NULL, NULL}
};

ObjModule* importStdLib(VM *vm, int index) {
    return libraries[index].libInitFunc(vm);
}

int getStdLib(VM* vm, const char* name, int length) {
    UNUSED(vm);
    for (int i = 0; libraries[i].name != NULL; i++) {
        if (!strncmp(libraries[i].name, name, length))
            return i;
    }
    return -1;
}
