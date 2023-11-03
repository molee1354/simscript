#ifndef simscript_compiler_h
#define simscript_compiler_h

#include "object.h"
#include "scanner.h"

/**
 * @brief Parser struct to hold the current and previous parser
 *
 */
typedef struct {
    VM* vm;
    Token current;
    Token previous;

    bool hadError;     // if the parser encountered an error during parsing
    bool panicMode;    // set panicMode to unwind out of the parser code
    ObjModule* module; // Module object
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_UNARY,       // ! -
    PREC_SUBSCRIPT,   // []
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

/**
 * @brief Struct for local variable
 *
 */
typedef struct {
    Token name;
    int depth;       // Depth of local variable

    bool isConst;    // if variable is declared constant
    bool isScoped;   // if variable is strictly local
    bool isCaptured; // true if local is captured by any later nested func dec
} Local;

typedef struct {
    int depth;
    bool isConst;
    bool isScoped;
} ResolvedVar;

/**
 * @brief Struct to hold upvalues
 *
 */
typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

/**
 * @brief Struct for holding Loops
 *
 */
typedef struct Loop {
    struct Loop* enclosing;
    int start;
    int body;
    int end;
    int scopeDepth;
} Loop;

/**
 * @class ClassCompiler
 * @brief Class compiler holding information about the current enclosing class
 *
 */
typedef struct ClassCompiler {
    struct ClassCompiler* enclosing;
    bool hasSuperClass;
} ClassCompiler;

/**
 * @brief Compiler struct
 *
 */
typedef struct Compiler {
    struct Compiler* enclosing;
    Parser* parser;                // Current parser

    Loop* loop;                    // Loop state

    ClassCompiler* klass;

    // Setting up an implicit top-level function
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;                // The number of local variables

    Upvalue upvalues[UINT8_COUNT]; // Upvalue array
    int scopeDepth;                // The depth of the scope (0 for global)
} Compiler;

/**
 * @brief ParseFn is a simple typedef for a function with no args and no return
 *
 */
typedef void (*ParseFn)(Compiler* compiler, bool canAssign);

/**
 * @brief Wrapper for a single row in the parser table
 *
 */
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

/**
 * @brief Method to compile source code
 *
 * @param source Source code from input stream
 * @param chunk Chunk to write to
 * @return True if the parser encountered an error
 */
ObjFunction* compile(VM* vm, ObjModule* module, const char* source);

/**
 * @brief Method to mark the compiler root
 *
 */
void markCompilerRoots(VM* vm);

#endif
