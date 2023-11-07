#ifndef simscript_stdlib_io_h
#define simscript_stdlib_io_h

#include "../library.h"

#define IO_DEF "\n" \
"use IO_base;\n"         \
"class stdout {\n"  \
"    print"

ObjModule* initLib_IO_base(VM* vm);

ObjModule* initLib_IO(VM* vm);

#endif
