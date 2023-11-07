#include <string.h>

#include "library.h"
#include "libs/io.h"

StdLib libraries[] = {
    {"IO", &initLib_IO},
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
