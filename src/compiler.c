#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "table.h"
#include "value.h"
#include "library.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#define REPL (parser->vm->repl)

/**
 * @brief Method to return the current chunk in compilation
 *
 * @return Chunk* The pointer to the current chunk in compilation
 */
static Chunk* currentChunk(Compiler* compiler) {
    return &compiler->function->chunk;
}

/**
 * @brief Method to handle an error and set out an error message
 * @param token Pointer to token with the error
 * @param message Error message
 *
 */
static void errorAt(Parser* parser, Token* token, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
#ifdef _WIN32
    fprintf(stderr, "\nCOMPILE ERROR:\n");
#else
    fprintf(stderr, "\n\033[1;31mCOMPILE ERROR:\033[0m\n");
#endif
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "  @ '%s', line %d\n",
                parser->module->name->chars,
                token->line);

    if (token->type==TOKEN_EOF) {
        fprintf(stderr, "  at end\n");
    } else if (token->type == TOKEN_ERROR) {
        // pass
    } else {
        fprintf(stderr, "  %.*s _ %.*s\n",
                parser->previous.length,
                parser->previous.start,
                parser->current.length,
                parser->current.start
                );
        for (int i=0; i < parser->previous.length+3; i++) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "%s\n", "^");
    }
    parser->hadError = true;
}

/**
 * @brief Method to handle an error that happened at a token we just consumed
 * @param message Error message
 *
 */
static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

/**
 * @brief Method to handle a parser error and set out an error message.
 * @param message Error message
 *
 */
static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

/**
 * @brief Method to advance in a stream of tokens
 *
 */
static void advance(Parser* parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = scanToken(&parser->scanner);
        if (parser->current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser, parser->current.start);
    }
}

/**
 * @brief Method to advance a token, validates it for an expected type, and 
 * reports an error if it doesn't encounter the correct type
 * @param type The expected token type of the current token.
 * @param message Error message to be emitted
 * 
 */
static void consume(Compiler* compiler, Tokentype type, const char* message) {
    if (compiler->parser->current.type==type) {
        advance(compiler->parser);
        return;
    }

    errorAtCurrent(compiler->parser, message);
}

/**
 * @brief Helper method to check if the current type is of a certain type
 *
 * @param type The type to match 
 * @return True if the types match
 */
static bool check(Compiler* compiler, Tokentype type) {
    return compiler->parser->current.type == type;
}

/**
 * @brief Method to check the current type going through the parser
 *
 * @param type The type to match
 * @return True if the types match
 */
static bool match(Compiler* compiler, Tokentype type) {
    if (!check(compiler, type)) return false;
    advance(compiler->parser);
    return true;
}

/**
 * @brief Method to append a single byte to the chunk
 * @param byte Byte to write
 *
 */
static void emitByte(Compiler* compiler, uint8_t byte) {
    writeChunk(compiler->parser->vm, currentChunk(compiler), byte, compiler->parser->previous.line);
}

/**
 * @brief Method to emit two bytes
 * @param byte1 Byte to write to chunk
 * @param byte2 Byte to write to chunk
 *
 */
static void emitBytes(Compiler* compiler, uint8_t byte1, uint8_t byte2) {
    emitByte(compiler, byte1);
    emitByte(compiler, byte2);
}

static void emitLoop(Compiler* compiler, int loopStart) {
    emitByte(compiler, OP_LOOP);

    int offset = currentChunk(compiler)->count - loopStart + 2;
    if (offset > UINT16_MAX) error(compiler->parser, "Loop body too large");

    emitByte(compiler, (offset>>8) & 0xff);
    emitByte(compiler, offset & 0xff);
}

/**
 * @brief Method to jump a certain offset for control flow
 *
 * @param instruction The instruction to jump
 * @return int The current chunk offset by a jump count
 */
static int emitJump(Compiler* compiler, uint8_t instruction) {
    emitByte(compiler, instruction);
    emitByte(compiler, 0xff);
    emitByte(compiler, 0xff);
    return currentChunk(compiler)->count-2;
}

/**
 * @brief Method to write OP_RETURN to the current chunk
 *
 */
static void emitReturn(Compiler* compiler) {
    if (compiler->type == TYPE_INITIALIZER) {
        emitBytes(compiler, OP_GET_LOCAL, 0);
    } else {
        emitByte(compiler, OP_NULL);
    }
    emitByte(compiler, OP_RETURN);
}

/**
 * @brief Method to insert an entry into the constant table
 * @param value Value to insert
 *
 */
static uint8_t makeConstant(Compiler* compiler, Value value) {
    int constant = addConstant(compiler->parser->vm, currentChunk(compiler), value);

    // store up to 256 constants in a chunk. Needs to be expanded
    if (constant > UINT8_MAX) {
        error(compiler->parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

/**
 * @brief Method to write a constant to the current chunk
 * @param value Value to write to current chunk
 *
 */
static void emitConstant(Compiler* compiler, Value value) {
    emitBytes(compiler, OP_CONSTANT, makeConstant(compiler, value));
}

static void patchJump(Compiler* compiler, int offset) {
    // -2 to adjust for the bytecode jump offset
    int jump = currentChunk(compiler)->count - offset -2;

    if (jump > UINT16_MAX) {
        error(compiler->parser, "Too much code to jump over");
    }
    currentChunk(compiler)->code[offset] = (jump >> 8) & 0xff;
    currentChunk(compiler)->code[offset+1] = jump & 0xff;
}

/**
 * @brief Constructor for the compiler struct. Zero initializes the values for
 * the compiler
 *
 * @param compiler The pointer to the compiler to be initialized
 */
static void initCompiler(Parser* parser, Compiler* compiler, Compiler* parent, FunctionType type) {
    compiler->parser = parser;
    compiler->enclosing = parent;

    compiler->function = NULL;
    if (parent != NULL) {
        compiler->klass = parent->klass;
    }

    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(parser->vm, parser->module, type);

    // storing the function's name (if not top-level/script)
    if (type != TYPE_SCRIPT) {
        compiler->function->name = copyString(parser->vm, parser->previous.start,
                                              parser->previous.length);
    }

    compiler->loop = NULL;

    // compiler implicitly claims stack slot 0 for VM use
    Local* local = &compiler->locals[compiler->localCount++];
    local->depth = compiler->scopeDepth;
    local->isConst = false;
    local->isScoped = false;
    local->isCaptured = false;
    if (type != TYPE_FUNCTION) {
        local->name.start = "this";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

/**
 * @brief Method to end the compiler by returning the function object
 *
 */
static ObjFunction* endCompiler(Compiler* compiler) {
    emitReturn(compiler);
    ObjFunction* function = compiler->function;
#ifdef DEBUG_PRINT_CODE
    if (!compiler->parser->hadError) {
        // pointing out where the error occurred. "<script>" if at
        // "main" function
        disassembleChunk( currentChunk(compiler), function->name != NULL ?
                function->name->chars : "<script>");
    }
#endif
    if (compiler->enclosing != NULL) {
        emitBytes(compiler->enclosing, OP_CLOSURE,
                  makeConstant(compiler->enclosing, OBJ_VAL(function)));

        /* OP_CLOSURE has variable byte size encoding -> 1 : local var
         *                                               0 : function upvalue
         *                                             int : upvalue index
         */
        for (int i=0; i < function->upvalueCount; i++) {
            emitByte(compiler->enclosing, compiler->upvalues[i].isLocal ? 1 : 0);
            emitByte(compiler->enclosing, compiler->upvalues[i].index);
        }
    }
    compiler->parser->vm->compiler = compiler->enclosing;
    return function;
}

/**
 * @brief Method to "go down" a single scope.
 *
 */
static void beginScope(Compiler* compiler) {
    compiler->scopeDepth++;
}

/**
 * @brief Method to "go up" a single scope.
 * 
 */
static void endScope(Compiler* compiler) {
    compiler->scopeDepth--;

    // destroying local vars at end of scope
    while (compiler->localCount > 0 &&
           compiler->locals[compiler->localCount-1].depth >
               compiler->scopeDepth) {
        if (compiler->locals[compiler->localCount-1].isCaptured) {
            emitByte(compiler, OP_CLOSE_UPVALUE);
        } else {
            emitByte(compiler, OP_POP);
        }
        compiler->localCount--;  // decrement count
    }
}

/* Wrapper function declarations */
static void expression(Compiler* compiler);
static void statement(Compiler* compiler);
static void declaration(Compiler* compiler);
static ParseRule* getRule(Tokentype type);
static void parsePrecedence(Compiler* compiler, Precedence precedence);

static uint8_t argumentList(Compiler* compiler);

/**
 * @brief Method to write the constant name as a string to the table
 *
 * @param name Name of the token
 * @return uint8_t index of the constant in the program
 */
static uint8_t identifierConstant(Compiler* compiler, Token* name) {
    return makeConstant(compiler, OBJ_VAL(copyString(compiler->parser->vm,
                                                     name->start,
                                                     name->length)));
}

/**
 * @brief Method check if two identifiers are the same
 *
 * @param a First identifier
 * @param b Second identifier
 * @return bool True if the identifiers are equal
 */
static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

/**
 * @brief Method to check if a given token name exists in the local scope
 *
 * @param compiler The current compiler
 * @param name The name of the token
 * @return static int The local index where the variable is found, -1 if not. 
 */
static ResolvedVar resolveLocal(Compiler* compiler, Token* name) {
    ResolvedVar out = {-1, false, false};
    for (int i = compiler->localCount -1; i>=0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error(compiler->parser, "Can't read local variable in its own initializer.");
            }
            out.depth = i;
            out.isConst = local->isConst;
            out.isScoped = local->isScoped;
            return out;
        }
    }
    return out; // TODO: Fix global scope errors
}

/**
 * @brief Method to add an upvalue to the upvalue array.
 *
 * @param compiler The current compiler
 * @param index The index of the upvalue
 * @param isLocal True if the current upvalue is that of a local variable
 * @return static int The upvalue count.
 */
static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal) {
    int upvalueCount = compiler->function->upvalueCount;

    /* seeing if there is already an upvalue in the array whose slot index
     * matches the one we're adding
     */
    for (int i=0; i<upvalueCount; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }
    if (upvalueCount == UINT8_COUNT) {
        error(compiler->parser, "Too many closure variables in a function");
        return 0;
    }
    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}
/**
 * @brief Method that looks for a local variable declared in any of the
 * surrounding functions.
 *
 * @param compiler The current compiler.
 * @param name The name of the token.
 * @return static int The upvalue index for that variable
 */
static ResolvedVar resolveUpvalue(Compiler* compiler, Token* name) {
    ResolvedVar out = {-1, false, false};
    // if (compiler->enclosing == NULL) return -1;
    if (compiler->enclosing == NULL) return out;

    int local = resolveLocal(compiler->enclosing, name).depth;
    bool isScoped = resolveLocal(compiler->enclosing, name).isScoped;
    bool isConst = resolveLocal(compiler->enclosing, name).isConst;

    // if (isScoped) return -1;
    if (isScoped) return out;

    if (local != -1) { // if local var is found
        // for resolving an identifier, mark it as "captured"
        compiler->enclosing->locals[local].isCaptured = true;
        // return addUpvalue(compiler, (uint8_t)local, true);
        out.depth = addUpvalue(compiler, (uint8_t)local, true);
        out.isConst = isConst;
        out.isScoped = isScoped; // should be false
        return out;
    }

    /* recursive use of resolveUpvalue to capture from its immediately
     * surrounding function (arg to addUpvalue is false). 
     * This is for "not top-level" 
     */
    int upvalue = resolveUpvalue(compiler->enclosing, name).depth;
    if (upvalue != -1) {
        // return addUpvalue(compiler, (uint8_t)upvalue, false);
        out.depth = addUpvalue(compiler, (uint8_t)upvalue, false);
        out.isConst = isConst;
        out.isScoped = isScoped; // should be false
        return out;
    }
    // return -1;
    return out;
}

/**
 * @brief Method to add a local variable to the compiler locals stack
 *
 * @param name Name of variable token.
 */
static void addLocal(Compiler* compiler, Token name, bool isConst, bool isScoped) {
    // checking for local variable max count
    if (compiler->localCount == UINT8_COUNT) {
        error(compiler->parser, "Too many variables in compiler scope.");
        return;
    }
    Local* local = &compiler->locals[compiler->localCount++];
    local->name = name;
    local->depth = -1;
    local->isConst = isConst;
    local->isScoped = isScoped;
    local->isCaptured = false;
}

/**
 * @brief Method to handle binary operations
 *
 */
static void binary(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    Tokentype operatorType = compiler->parser->previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence(compiler, (Precedence)(rule->precedence + 1) );

    switch (operatorType) {
        // logic oper
        case TOKEN_BANG_EQUAL:    emitBytes(compiler, OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(compiler, OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(compiler, OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(compiler, OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(compiler, OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(compiler, OP_GREATER, OP_NOT); break;

        // math oper
        case TOKEN_PLUS:          emitByte(compiler, OP_ADD); break;
        case TOKEN_MINUS:         emitByte(compiler, OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(compiler, OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(compiler, OP_DIVIDE); break;
        case TOKEN_MOD:           emitByte(compiler, OP_MOD); break;
        case TOKEN_POWER:         emitByte(compiler, OP_POWER); break;

        // unreachable
        default: return;
    }
}

/**
 * @brief Method to parse function calls
 *
 * @param canAssign True if assignable
 */
static void call(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    uint8_t argCount = argumentList(compiler);
    emitBytes(compiler, OP_CALL, argCount);
}

/**
 * @brief Method to call fields on class instances
 *
 * @param canAssign True if assignable
 */
static void dot(Compiler* compiler, bool canAssign) {
    consume(compiler, TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(compiler, &compiler->parser->previous);

    if (canAssign && match(compiler, TOKEN_EQUAL)) {
        expression(compiler);
        emitBytes(compiler, OP_SET_PROPERTY, name);

    } else if (canAssign && match(compiler, TOKEN_PLUS_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        expression(compiler);
        emitByte(compiler, OP_ADD);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (canAssign && match(compiler, TOKEN_MINUS_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        expression(compiler);
        emitByte(compiler, OP_SUBTRACT);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (canAssign && match(compiler, TOKEN_STAR_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        expression(compiler);
        emitByte(compiler, OP_MULTIPLY);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (canAssign && match(compiler, TOKEN_SLASH_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        expression(compiler);
        emitByte(compiler, OP_DIVIDE);
        emitBytes(compiler, OP_SET_PROPERTY, name);

    } else if (canAssign && match(compiler, TOKEN_PLUS_PLUS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        emitByte(compiler, OP_INCREMENT);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (canAssign && match(compiler, TOKEN_MINUS_MINUS)) {
        emitBytes(compiler, OP_GET_PROPERTY_NOPOP, name);
        emitByte(compiler, OP_DECREMENT);
        emitBytes(compiler, OP_SET_PROPERTY, name);

    } else if (match(compiler, TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList(compiler);
        emitBytes(compiler, OP_INVOKE, name);
        emitByte(compiler, argCount);
    } else {
        emitBytes(compiler, OP_GET_PROPERTY, name);
    }
}

/**
 * @brief Method to parse literals
 *
 * @param canAssign True if assignable
 */
static void literal(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    switch (compiler->parser->previous.type) {
        case TOKEN_FALSE:  emitByte(compiler, OP_FALSE); break;
        case TOKEN_NULL:   emitByte(compiler, OP_NULL); break;
        case TOKEN_TRUE:   emitByte(compiler, OP_TRUE); break;

        // unreachable
        default: return;
    }
}

/**
 * @brief Method to handle grouping by parenthesis. Recursively call
 * expression() to handle expressions within the parenthesis
 *
 */
static void grouping(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    expression(compiler); // takes care of generating bytecode inside the parenthesis
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * @brief Method to convert a parsed string to a number
 *
 */
static void number(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    double value = strtod(compiler->parser->previous.start, NULL);
    emitConstant(compiler, NUMBER_VAL(value));
}

/**
 * @brief Method to handle the 'and' logical operation
 *
 * @param canAssign Variable to check if the value can be assigned
 */
static void and_(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    int endJump = emitJump(compiler, OP_JUMP_IF_FALSE);

    emitByte(compiler, OP_POP);
    parsePrecedence(compiler, PREC_AND);
    patchJump(compiler, endJump);
}

/**
 * @brief Method to handle the 'or' logical operation
 *
 * @param canAssign Variable to check if the value can be assigned
 */
static void or_(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    int elseJump = emitJump(compiler, OP_JUMP_IF_FALSE);
    int endJump = emitJump(compiler, OP_JUMP);

    patchJump(compiler, elseJump);
    emitByte(compiler, OP_POP);

    parsePrecedence(compiler, PREC_OR);
    patchJump(compiler, endJump);
}

static int parseEscapeSequence(Parser *parser, char *string, int length) {
    UNUSED(parser);
    for (int i = 0; i < length - 1; i++) {
        if (string[i] == '\\') {
            switch (string[i + 1]) {
                case 'n': {
                    string[i + 1] = '\n';
                    break;
                }
                case 't': {
                    string[i + 1] = '\t';
                    break;
                }
                case 'r': {
                    string[i + 1] = '\r';
                    break;
                }
                case 'v': {
                    string[i + 1] = '\v';
                    break;
                }
                case '\\': {
                    string[i + 1] = '\\';
                    break;
                }
                case '\'':
                case '"': {
                    break;
                }
                default: {
                    continue;
                }
            }
            memmove(&string[i], &string[i + 1], length - i);
            length -= 1;
        }
    }

    return length;
}

static void rawString(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    if (match(compiler, TOKEN_STRING)) {
        Parser* parser = compiler->parser;
        emitConstant(compiler, OBJ_VAL(copyString(parser->vm,
                                                  parser->previous.start+1,
                                                  parser->previous.length-2)));
        return ;
    }
    consume(compiler, TOKEN_STRING, "Expected raw string after single quote opening.");
}

static Value parseString(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);

    Parser* parser = compiler->parser;
    int strLen = parser->previous.length-2;
    char* string = ALLOCATE(parser->vm, char, strLen+1);

    memcpy(string, parser->previous.start+1, strLen);
    int length = parseEscapeSequence(parser, string, strLen);

    if (length != strLen) {
        string = GROW_ARRAY(parser->vm, char, string, strLen+1, length+1);
    }
    string[length] = '\0';
    return OBJ_VAL(takeString(parser->vm, string, length));
}

/**
 * @brief Method to convert a parsed string into string value
 *
 */
static void string(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    emitConstant(compiler, parseString(compiler, canAssign));
}

/**
 * @brief TODO add comment here
 *
 * @param name 
 */
static void namedVariable(Compiler* compiler, Token name, bool canAssign) {
    uint8_t getOp, setOp;
    ResolvedVar resLoc = resolveLocal(compiler, &name);
    int arg = resLoc.depth;
    bool isConst = resLoc.isConst;
    bool isScoped = resLoc.isScoped;

    if (arg != -1) { // -1 for global state
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ( !isScoped &&
                (arg = resolveUpvalue(compiler, &name).depth) != -1) {
        isConst = resolveUpvalue(compiler, &name).isConst;
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(compiler, &name);
        /* getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL; */
        ObjString* string = copyString(compiler->parser->vm, name.start, name.length);
        Value value;
        if (tableGet(&compiler->parser->vm->globals, string, &value)) {
            getOp = OP_GET_GLOBAL;
            canAssign = false;
        } else {
            getOp = OP_GET_MODULE;
            setOp = OP_SET_MODULE;
        }
    }

    if (canAssign && match(compiler, TOKEN_EQUAL)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        expression(compiler);
        emitBytes(compiler, setOp, (uint8_t)arg);

    } else if (canAssign && match(compiler, TOKEN_PLUS_PLUS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        emitByte(compiler, OP_INCREMENT);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else if (canAssign && match(compiler, TOKEN_MINUS_MINUS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        emitByte(compiler, OP_DECREMENT);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else if (canAssign && match(compiler, TOKEN_PLUS_EQUALS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        expression(compiler);
        emitByte(compiler, OP_ADD);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else if (canAssign && match(compiler, TOKEN_MINUS_EQUALS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        expression(compiler);
        emitByte(compiler, OP_SUBTRACT);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else if (canAssign && match(compiler, TOKEN_STAR_EQUALS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        expression(compiler);
        emitByte(compiler, OP_MULTIPLY);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else if (canAssign && match(compiler, TOKEN_SLASH_EQUALS)) {
        if (isConst) {
            error(compiler->parser, "Cannot reassign values to constants.");
        }
        namedVariable(compiler, name, false);
        expression(compiler);
        emitByte(compiler, OP_DIVIDE);
        emitBytes(compiler, setOp, (uint8_t)arg);
    } else {
        emitBytes(compiler, getOp, (uint8_t)arg);
    }
}

/**
 * @brief Method to parse variables
 *
 */
static void variable(Compiler* compiler, bool canAssign) {
    namedVariable(compiler, compiler->parser->previous, canAssign);
}

/**
 * @brief Method to create a token on demand
 *
 * @param text The name value of token
 * @return Token Token with the name text
 */
static Token syntheticToken(const char* text) {
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void super_(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    ClassCompiler* currentClass = compiler->klass;
    // limiting the use of super
    if (currentClass == NULL) {
        error(compiler->parser, "Can't use 'super' outside of a class.");
    } else if (!currentClass->hasSuperClass) {
        error(compiler->parser, "Can't use 'super' in a class with no parent.");
    }

    consume(compiler, TOKEN_DOT, "Expect '.' after 'super'.");
    consume(compiler, TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(compiler, &compiler->parser->previous);

    namedVariable(compiler, syntheticToken("this"), false);

    // Differentiating "invocations" and "gets"
    if (match(compiler, TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList(compiler);
        namedVariable(compiler, syntheticToken("super"), false);
        emitBytes(compiler, OP_SUPER_INVOKE, name);
        emitByte(compiler, argCount);
    } else {
        namedVariable(compiler, syntheticToken("super"), false);
        emitBytes(compiler, OP_GET_SUPER, name);
    }
}

/**
 * @brief Method to parse the 'this' keyword. Treated as a lexically scoped
 * local variable whose value gets initialized. 
 *
 * @param canAssign 
 */
static void this_(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    if (compiler->klass == NULL) {
        error(compiler->parser, "Using 'this' out of a classdef context.");
        return;
    }
    variable(compiler, false);
}

/**
 * @brief Method to deal with the unary minus
 *
 */
static void unary(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    Tokentype operatorType = compiler->parser->previous.type;

    // compiling the operand
    parsePrecedence(compiler, PREC_UNARY);

    // emit operator instruction
    switch (operatorType) {
        case TOKEN_BANG:  emitByte(compiler, OP_NOT); break;
        case TOKEN_MINUS: emitByte(compiler, OP_NEGATE); break;
        default: return; // unreachable
    }
}

/**
 * @brief Method to emit the increment operation
 *
 * @param compiler 
 * @param canAssign 
 */
static void increment(Compiler* compiler,bool canAssign) {
    UNUSED(canAssign);
    emitByte(compiler, OP_INCREMENT);
}

/**
 * @brief Method to emit the decrement operation
 *
 * @param compiler 
 * @param canAssign 
 */
static void decrement(Compiler* compiler,bool canAssign) {
    UNUSED(canAssign);
    emitByte(compiler, OP_DECREMENT);
}

static void list(Compiler* compiler, bool canAssign) {
    UNUSED(canAssign);
    int numElem = 0;
    if (!check(compiler, TOKEN_RIGHT_BRACKET)) {
        do {
            if (check(compiler, TOKEN_RIGHT_BRACKET))
                break;
            parsePrecedence(compiler, PREC_OR);
            numElem++;
        } while(match(compiler, TOKEN_COMMA));
    }
    consume(compiler, TOKEN_RIGHT_BRACKET, "Expected ']' at list end.");
    emitBytes(compiler, OP_MAKE_LIST, numElem);
}

static void subscript(Compiler* compiler, bool canAssign) {
    parsePrecedence(compiler, PREC_OR);

    consume(compiler, TOKEN_RIGHT_BRACKET, "Expected ']' after subscript.");
    if (canAssign && match(compiler, TOKEN_EQUAL)) {
        expression(compiler);
        emitByte(compiler, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(compiler, TOKEN_PLUS_EQUALS)) {
        emitByte(compiler, OP_SUBSCRIPT_IDX_NOPOP);
        expression(compiler);
        emitBytes(compiler, OP_ADD, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(compiler, TOKEN_MINUS_EQUALS)) {
        emitByte(compiler, OP_SUBSCRIPT_IDX_NOPOP);
        expression(compiler);
        emitBytes(compiler, OP_SUBTRACT, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(compiler, TOKEN_STAR_EQUALS)) {
        emitByte(compiler, OP_SUBSCRIPT_IDX_NOPOP);
        expression(compiler);
        emitBytes(compiler, OP_MULTIPLY, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(compiler, TOKEN_SLASH_EQUALS)) {
        emitByte(compiler, OP_SUBSCRIPT_IDX_NOPOP);
        expression(compiler);
        emitBytes(compiler, OP_DIVIDE, OP_SUBSCRIPT_ASSIGN);

    } else if (canAssign && match(compiler, TOKEN_PLUS_PLUS)) {
        emitBytes(compiler, OP_SUBSCRIPT_IDX_NOPOP, OP_INCREMENT);
        emitByte(compiler, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(compiler, TOKEN_MINUS_MINUS)) {
        emitBytes(compiler, OP_SUBSCRIPT_IDX_NOPOP, OP_DECREMENT);
        emitByte(compiler, OP_SUBSCRIPT_ASSIGN);
    } else {
        emitByte(compiler, OP_SUBSCRIPT_IDX);
    }
}

/**
 * @brief The array of parse rules
 *
 */
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACKET]  = {list,     subscript,   PREC_SUBSCRIPT}, 
    [TOKEN_RIGHT_BRACKET] = {NULL,     NULL,   PREC_NONE}, 

    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     dot,   PREC_CALL},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
    [TOKEN_POWER]         = {NULL,     binary, PREC_POWER},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_MOD]           = {NULL,     binary, PREC_FACTOR},
    [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_RSTRING]       = {rawString,NULL, PREC_NONE},

    [TOKEN_PLUS_PLUS]     = {NULL,     increment,   PREC_NONE},
    [TOKEN_PLUS_EQUALS]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS_MINUS]   = {NULL,     decrement,   PREC_NONE},
    [TOKEN_MINUS_EQUALS]  = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STAR_EQUALS]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH_EQUALS]  = {NULL,     NULL,   PREC_NONE},

    [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
    [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
    [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
    [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NULL]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
    [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
    [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LOCAL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_CONST]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

/**
 * @brief Starts at current token, and parses any expression at the given
 * precedence level or higher.
 *
 */
static void parsePrecedence(Compiler* compiler, Precedence precedence) {
    advance(compiler->parser);
    ParseFn prefixRule = getRule(compiler->parser->previous.type)->prefix;

    if (prefixRule==NULL) {
        error(compiler->parser, "Expect expression.");
        return;
    }

    // checking if assignment can happen
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(compiler, canAssign);

    while (precedence <= getRule(compiler->parser->current.type)->precedence) {
        advance(compiler->parser);
        ParseFn infixRule = getRule(compiler->parser->previous.type)->infix;
        infixRule(compiler, canAssign);
    }

    if (canAssign && match(compiler, TOKEN_EQUAL)) {
        error(compiler->parser, "Invalid assignment target.");
    }
}


/**
 * @brief Method to declare and add a local variable.
 * 
 */
static void declareVariable(Compiler* compiler, bool isConst, bool isScoped) {
    if (compiler->scopeDepth == 0) return; // if at global scope, return

    Token* name = &compiler->parser->previous;
    for (int i = compiler->localCount-1; i>=0; i--) {
        Local* local = &compiler->locals[i];
        if (local->depth != -1 && local->depth < compiler->scopeDepth) {
            break;
        }

        // checking for var name collisions in local scope
        if (identifiersEqual(name, &local->name)) {
            error(compiler->parser, "Already a variable with this name in same scope.");
        }
    }
    addLocal(compiler, *name, isConst, isScoped);
}

/**
 * @brief Method to parse a variable declaration and expect an identifier
 *
 * @param errorMessage Error message to emit when no identifier is found
 * @return uint8_t index of the constant in the constant array
 */
static uint8_t parseVariable(Compiler* compiler,
                             const char* errorMessage,
                             bool isConst, bool isScoped) {
    consume(compiler, TOKEN_IDENTIFIER, errorMessage);

    declareVariable(compiler, isConst, isScoped);
    if (compiler->scopeDepth > 0) return 0; // exit the function if local scope

    return identifierConstant(compiler, &compiler->parser->previous);
}

/**
 * @brief Method to mark a variable as initialized.
 */
static void markInitialized(Compiler* compiler) {
    if (compiler->scopeDepth == 0) return;
    compiler->locals[compiler->localCount - 1].depth = compiler->scopeDepth;
}

/**
 * @brief Method to output the bytecode instruction that defines the new
 * variable and stores its initial value
 *
 * @param global Variable index
 */
static void defineVariable(Compiler* compiler, uint8_t global) {
    if (compiler->scopeDepth == 0) {
        emitBytes(compiler, OP_DEFINE_MODULE, global);
    } else {
        // Mark the local as defined now.
        compiler->locals[compiler->localCount - 1].depth = compiler->scopeDepth;
    }

    // don't define var if in local scope
    /* if (compiler->scopeDepth > 0) {
        markInitialized(compiler);
        return;
    }
    emitBytes(compiler, OP_DEFINE_GLOBAL, global); */
}

/**
 * @brief Method to compile the arguments passed into the function. Go through
 * the argument list as long as we find commas after the expression.
 *
 * @return uint8_t Number of arguments passed into the function
 */
static uint8_t argumentList(Compiler* compiler) {
    uint8_t argCount = 0;
    if (!check(compiler, TOKEN_RIGHT_PAREN)) {
        do {
            expression(compiler);
            if (argCount >= 255) error(compiler->parser, "Can't have more than 255 arguments.");
            argCount++;
        } while (match(compiler, TOKEN_COMMA));
    }
    consume(compiler, TOKEN_RIGHT_PAREN, "Missing ')' or ',' in function argument input.");
    return argCount;
}

/**
 * @brief Method to parse blocks inside braces. Expects a closing curly brace
 *
 */
static void block(Compiler* compiler) {
    while (!check(compiler, TOKEN_RIGHT_BRACE) && !check(compiler, TOKEN_EOF)) {
        declaration(compiler);
    }

    consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * @brief A method that indexes the parsing rules table based on type. The
 * lookup is wrapped in a function.
 *
 */
static ParseRule* getRule(Tokentype type) {
    return &rules[type];
}

/**
 * @brief Method to parse expressions
 *
 */
static void expression(Compiler* compiler) {
    parsePrecedence(compiler, PREC_ASSIGNMENT);
}

/**
 * @brief Method to compile the function. A separate compiler is created for
 * each function being compiled.
 *
 * @param type The type of function to compile
 */
static void beginFunction(Compiler* compiler, Compiler* funcCompiler, FunctionType type) {
    initCompiler(compiler->parser, funcCompiler, compiler, type);
    beginScope(funcCompiler);

    consume(funcCompiler, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    // parameters
    if (!check(funcCompiler, TOKEN_RIGHT_PAREN)) {
        do {
            funcCompiler->function->params++;
            if (funcCompiler->function->params > 255) {
                errorAtCurrent(funcCompiler->parser, "Can't have more than 255 parameters");
            }
            uint8_t constant = parseVariable(funcCompiler, "Expect parameter name", false, false);
            defineVariable(funcCompiler, constant);
        } while (match(funcCompiler, TOKEN_COMMA));
    }
    consume(funcCompiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
}

/**
 * @brief Method to compile the function
 *
 * @param compiler The current compiler
 * @param type The type of the function
 */
static void function(Compiler* compiler, FunctionType type) {
    Compiler funcCompiler;

    beginFunction(compiler, &funcCompiler, type);

    consume(&funcCompiler, TOKEN_LEFT_BRACE, "Expect '{' before function body");
    block(&funcCompiler);

    endCompiler(&funcCompiler);
}

/**
 * @brief Method to add a class method
 */
static void method(Compiler* compiler) {
    consume(compiler, TOKEN_IDENTIFIER, "Expect method name");
    uint8_t constant = identifierConstant(compiler, &compiler->parser->previous);
    FunctionType type = TYPE_METHOD;
    if ( compiler->parser->previous.length == 4 &&
        memcmp(compiler->parser->previous.start, "init", 4) == 0 ) {
        type = TYPE_INITIALIZER;
    }
    function(compiler, type);
    emitBytes(compiler, OP_METHOD, constant);
}

static void setupClassCompiler(Compiler* compiler, ClassCompiler* classCompiler) {
    classCompiler->hasSuperClass = false;
    classCompiler->enclosing = compiler->klass;
    compiler->klass = classCompiler;
}

static void endClassCompiler(Compiler* compiler, ClassCompiler* classCompiler) {
    UNUSED(classCompiler);
    compiler->klass = compiler->klass->enclosing;
}
/**
 * @brief Method to parse class declarations
 */
static void classDeclaration(Compiler* compiler) {
    consume(compiler, TOKEN_IDENTIFIER, "Expect class name.");

    Token className = compiler->parser->previous;
    uint8_t nameConstant = identifierConstant(compiler, &compiler->parser->previous);
    declareVariable(compiler, false, false);

    emitBytes(compiler, OP_CLASS, nameConstant);
    defineVariable(compiler, nameConstant);

    ClassCompiler classCompiler;
    setupClassCompiler(compiler, &classCompiler);

    if (match(compiler, TOKEN_INHERIT)) {
        consume(compiler, TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(compiler, false);

        if (identifiersEqual(&className, &compiler->parser->previous)) {
            error(compiler->parser, "A class can't inherit from itself.");
        }

        beginScope(compiler);
        addLocal(compiler, syntheticToken("super"), false, false);
        defineVariable(compiler, 0);

        namedVariable(compiler, className, false);
        emitByte(compiler, OP_INHERIT);
        classCompiler.hasSuperClass = true;
    }

    namedVariable(compiler, className, false);
    consume(compiler, TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(compiler, TOKEN_RIGHT_BRACE) && !check(compiler, TOKEN_EOF)) {
        method(compiler);
    }
    consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(compiler, OP_POP);
    if (classCompiler.hasSuperClass) {
        endScope(compiler);
    }
    endClassCompiler(compiler, &classCompiler);
}

/**
 * @brief Method to parse function declarations as a first class value.
 */
static void funDeclaration(Compiler* compiler) {
    uint8_t global = parseVariable(compiler, "Expect function name.", false, false);
    markInitialized(compiler);
    function(compiler, TYPE_FUNCTION);
    defineVariable(compiler, global);
}

// compiler, "message", isConst, isScoped
/**
 * @brief A method to handle variable declarations.
 */
static void varDeclaration(Compiler* compiler, bool isScoped) {
    uint8_t global = parseVariable(compiler, "Expect variable name.", false, isScoped);

    if (match(compiler, TOKEN_EQUAL)) {
        expression(compiler);
    } else {
        emitByte(compiler, OP_NULL);
    }
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(compiler, global);
}

static void constDeclaration(Compiler* compiler, bool isScoped) {
    uint8_t global = parseVariable(compiler, "Expect variable name.", true, isScoped);

    if (!match(compiler, TOKEN_EQUAL)) {
        error(compiler->parser, "Constant declarations must be followed by a value assignment.");
    } else {
        expression(compiler);
    }
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after constant declaration");

    defineVariable(compiler, global);
}

/**
 * @brief Method to compile an expression followed by a semicolon.
 *
 */
static void expressionStatement(Compiler* compiler) {
    expression(compiler);
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(compiler, OP_POP);
}

/**
 * @brief Method to get the argument count of a given OpCode
 *
 * @param code 
 * @param constants 
 * @param ip 
 * @return 
 */
static int getArgCount(const uint8_t *code, const ValueArray constants, int ip) {
    switch (code[ip]) {
        case OP_NULL:
        case OP_TRUE:
        case OP_FALSE:
        case OP_POP:
        case OP_EQUAL:
        case OP_GREATER:
        case OP_LESS:
        case OP_ADD:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MOD:
        case OP_POWER:
        case OP_NOT:
        case OP_NEGATE:
        case OP_CLOSE_UPVALUE:
        case OP_RETURN:
        case OP_END_CLASS:
        case OP_BREAK:
        case OP_INCREMENT:
        case OP_DECREMENT:
        case OP_MODULE_VAR:
        case OP_MODULE_END:
        case OP_MAKE_LIST:
        case OP_SUBSCRIPT_IDX:
        case OP_SUBSCRIPT_IDX_NOPOP:
        case OP_SUBSCRIPT_ASSIGN:
            return 0;

        case OP_CONSTANT:
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
        case OP_GET_GLOBAL:
        case OP_GET_MODULE:
        case OP_SET_MODULE:
        case OP_DEFINE_MODULE:
        case OP_GET_UPVALUE:
        case OP_SET_UPVALUE:
        case OP_GET_PROPERTY:
        case OP_SET_PROPERTY:
        case OP_GET_SUPER:
        case OP_METHOD:
        case OP_MODULE:
            return 1;

        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_LOOP:
        case OP_CLASS:
        case OP_INHERIT:
        case OP_CALL:
        case OP_MODULE_BUILTIN:
            return 2;

        case OP_INVOKE:
        case OP_SUPER_INVOKE:
            return 3;

        case OP_CLOSURE: {
            int constant = code[ip + 1];
            ObjFunction* loadedFn = AS_FUNCTION(constants.values[constant]);

            // There is one byte for the constant, then two for each upvalue.
            return 1 + (loadedFn->upvalueCount * 2);
        }
    }

    return 0;
}
/**
 * @brief Method to end a loop
 *
 */
static void endLoop(Compiler* compiler) {
    if (compiler->loop->end != -1) {
        patchJump(compiler, compiler->loop->end);
        emitByte(compiler, OP_POP);
    }

    int i = compiler->loop->body;
    while (i < compiler->function->chunk.count) {
        if (compiler->function->chunk.code[i] == OP_BREAK) {
            compiler->function->chunk.code[i] = OP_JUMP;
            patchJump(compiler, i+1);
            i+=3;
        } else {
            i += 1 + getArgCount(compiler->function->chunk.code,
                                 compiler->function->chunk.constants, i);
        }
    }
    compiler->loop = compiler->loop->enclosing;
}
/**
 * @brief Method to parse through for statements
 */
static void forStatement(Compiler* compiler) {
    beginScope(compiler);
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // init clause
    if(match(compiler, TOKEN_SEMICOLON)) {
        // no init
    } else if (match(compiler, TOKEN_VAR)) {
        varDeclaration(compiler, false);
    } else {
        expressionStatement(compiler);
    }

    Loop loop;
    loop.start = currentChunk(compiler)->count;
    loop.scopeDepth = compiler->scopeDepth;
    loop.enclosing = compiler->loop;
    compiler->loop = &loop;

    // condition clause
    compiler->loop->end = -1;
    if (!match(compiler, TOKEN_SEMICOLON)) {
        expression(compiler);
        consume(compiler, TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // exit loop if condition is false
        compiler->loop->end = emitJump(compiler, OP_JUMP_IF_FALSE);
        emitByte(compiler, OP_POP);
    }

    // increment clause
    if (!match(compiler, TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(compiler, OP_JUMP);
        int incrementStart = currentChunk(compiler)->count;
        expression(compiler);
        emitByte(compiler, OP_POP);
        consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(compiler, compiler->loop->start);
        compiler->loop->start = incrementStart;
        patchJump(compiler, bodyJump);
    }
    
    compiler->loop->body = compiler->function->chunk.count;
    statement(compiler);
    emitLoop(compiler, compiler->loop->start);

    endLoop(compiler);
    endScope(compiler);
}

static void breakStatement(Compiler* compiler) {
    if (compiler->loop == NULL) {
        error(compiler->parser, "'break' statements can only be used in a loop.");
        return;
    }

    consume(compiler, TOKEN_SEMICOLON, "Expected ';' after 'break'.");

    // getting rid of locals in the loop scope
    for (int i = compiler->localCount - 1;
         i >= 0 && compiler->locals[i].depth > compiler->loop->scopeDepth;
         i--) {
        if (compiler->locals[i].isCaptured) {
            emitByte(compiler, OP_CLOSE_UPVALUE);
        } else {
            emitByte(compiler, OP_POP);
        }
    }
    emitJump(compiler, OP_BREAK);
}

static void continueStatement(Compiler* compiler) {
    if (compiler->loop == NULL) {
        error(compiler->parser, "'continue' statements can only be used in a loop.");
        return;
    }

    consume(compiler, TOKEN_SEMICOLON, "Expected ';' after 'continue'.");

    // getting rid of locals in the loop scope
    for (int i = compiler->localCount - 1;
         i >= 0 && compiler->locals[i].depth > compiler->loop->scopeDepth;
         i--) {
        if (compiler->locals[i].isCaptured) {
            emitByte(compiler, OP_CLOSE_UPVALUE);
        } else {
            emitByte(compiler, OP_POP);
        }
    }

    // go back to the compiler loop start
    emitLoop(compiler, compiler->loop->start);
}

/**
 * @brief Method to parse through the if statement
 */
static void ifStatement(Compiler* compiler) {
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after 'if'.");

    int thenJump = emitJump(compiler, OP_JUMP_IF_FALSE);
    emitByte(compiler, OP_POP);
    statement(compiler);

    int elseJump = emitJump(compiler, OP_JUMP);
    patchJump(compiler, thenJump);
    emitByte(compiler, OP_POP);

    if (match(compiler, TOKEN_ELSE)) statement(compiler);
    patchJump(compiler, elseJump);
}

/**
 * @brief Method to handle print statements
 */
static void printStatement(Compiler* compiler) {
    expression(compiler);
    if (compiler->parser->vm->repl) {
        emitByte(compiler, OP_PRINT);
        return;
    }
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after 'echo' argument.");
    emitByte(compiler, OP_PRINT);
}

static void import(Compiler* compiler) {
    int importIndex = makeConstant(compiler,
            OBJ_VAL(copyString(
                    compiler->parser->vm,
                    compiler->parser->previous.start + 1,
                    compiler->parser->previous.length - 2
                    )));
    emitBytes(compiler, OP_MODULE, importIndex);
    emitByte(compiler, OP_POP);
}

static void useStatement(Compiler* compiler) {
    consume(compiler, TOKEN_IDENTIFIER, "Expect library name after 'use'.");
    uint8_t libVarName = identifierConstant(compiler, &compiler->parser->previous);
    declareVariable(compiler, true, false);

    int idx = getStdLib(compiler->parser->vm,
                        compiler->parser->previous.start,
                        compiler->parser->previous.length - compiler->parser->current.length);
    if (idx == -1)
        error(compiler->parser, "Invalid library name.");

    emitBytes(compiler, OP_MODULE_BUILTIN, idx);
    emitByte(compiler, libVarName);
    defineVariable(compiler, libVarName);
    emitByte(compiler, OP_MODULE_END);
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after module import");
   
}

/**
 * @brief Method to handle module statements
 */
static void moduleStatement(Compiler* compiler) {
    if (match(compiler, TOKEN_STRING)) {
        import(compiler);
    } else if (check(compiler, TOKEN_IDENTIFIER)) {
        uint8_t moduleVarName = parseVariable(compiler, "Expect import namespace", false, false);
        consume(compiler, TOKEN_EQUAL, "Missing assignment '=' to module variable");
        if (!match(compiler, TOKEN_STRING)) {
            errorAtCurrent(compiler->parser, "Expect module path after '='.");
            return;
        }
        // consume(compiler, TOKEN_STRING, "Expect module path after '='.");
        import(compiler);
        emitByte(compiler, OP_MODULE_VAR);
        defineVariable(compiler, moduleVarName);
    }
    emitByte(compiler, OP_MODULE_END);
    consume(compiler, TOKEN_SEMICOLON, "Expect ';' after module import");
}

/**
 * @brief Method to handle return statements
 */
static void returnStatement(Compiler* compiler) {
    if (compiler->type == TYPE_SCRIPT) {
        error(compiler->parser, "Cannot return from top-level code.");
    }
    if (match(compiler, TOKEN_SEMICOLON)) {
        emitReturn(compiler);
    } else {
        if (compiler->type == TYPE_INITIALIZER) {
            error(compiler->parser, "Invalid attempt to return from an initializer.");
        }
        expression(compiler);
        consume(compiler, TOKEN_SEMICOLON, "Expected ';' after return statement.");
        emitByte(compiler, OP_RETURN);
    }
}

/**
 * @brief Method to handle while statements
 */
static void whileStatement(Compiler* compiler) {
    Loop loop;
    loop.start = currentChunk(compiler)->count;
    loop.scopeDepth = compiler->scopeDepth;
    loop.enclosing = compiler->loop;
    compiler->loop = &loop;

    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after 'while'.");

    compiler->loop->end = emitJump(compiler, OP_JUMP_IF_FALSE);
    emitByte(compiler, OP_POP);
    compiler->loop->body = compiler->function->chunk.count;
    statement(compiler);

    emitLoop(compiler, loop.start);
    endLoop(compiler);
}

/**
 * @brief Method to synchronize panic mode. Allows the compiler to exit
 * panic mode when it reaches a synchronization point.
 */
static void synchronize(Compiler* compiler) {
    compiler->parser->panicMode = false;

    while (compiler->parser->current.type != TOKEN_EOF) {
        if (compiler->parser->previous.type == TOKEN_SEMICOLON) return;
        switch (compiler->parser->current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_CONST:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_BREAK:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
            case TOKEN_MODULE:
                return;

            default:
                ; // do nothing
        }
        advance(compiler->parser);
    }
}

/**
 * @brief Method to compile a single declaration
 *
 */
static void declaration(Compiler* compiler) {
    if (match(compiler, TOKEN_CLASS)) {
        classDeclaration(compiler);
    } else if (match(compiler, TOKEN_FUN)) {
        funDeclaration(compiler);
    } else if (match(compiler, TOKEN_VAR)) {
        varDeclaration(compiler, false);
    } else if (match(compiler, TOKEN_CONST)) {
        constDeclaration(compiler, false);
    } else if (match(compiler, TOKEN_LOCAL)) {
        if (match(compiler, TOKEN_VAR)) {
            varDeclaration(compiler, true);
        } else if (match(compiler, TOKEN_CONST)) {
            constDeclaration(compiler, true);
        } else {
            error(compiler->parser,"Expected variable declaration after 'local'.");
        }
    } else {
        statement(compiler);
    }

    // if parser gets into panic mode, we synchronize
    if (compiler->parser->panicMode) synchronize(compiler);
}

/**
 * @brief Method to match statements according to their tokens
 * 
 */
static void statement(Compiler* compiler) {
    if (match(compiler, TOKEN_PRINT)) {
        printStatement(compiler);
    } else if (match(compiler, TOKEN_USING)) {
        useStatement(compiler);
    } else if (match(compiler, TOKEN_MODULE)) {
        moduleStatement(compiler);
    } else if (match(compiler, TOKEN_FOR)) {
        forStatement(compiler);
    } else if (match(compiler, TOKEN_IF)) {
        ifStatement(compiler);
    } else if (match(compiler, TOKEN_RETURN)) {
        returnStatement(compiler);
    } else if (match(compiler, TOKEN_WHILE)) {
        whileStatement(compiler);
    } else if (match(compiler, TOKEN_BREAK)) {
        breakStatement(compiler);
    } else if (match(compiler, TOKEN_CONTINUE)) {
        continueStatement(compiler);
    } else if (match(compiler, TOKEN_LEFT_BRACE)) {
        beginScope(compiler);
        block(compiler);
        endScope(compiler);
    } else {
        expressionStatement(compiler);
    }
}

ObjFunction* compile(VM* vm, ObjModule* module, const char *source) {
    Parser parser;
    parser.vm = vm;
    parser.hadError = false;
    parser.panicMode = false;
    parser.module = module;

    Scanner scanner;
    initScanner(&scanner, source);
    parser.scanner = scanner;

    Compiler compiler;
    initCompiler(&parser, &compiler, NULL, TYPE_SCRIPT);

    advance(compiler.parser);
    while (!match(&compiler, TOKEN_EOF)) {
        declaration(&compiler);
    }
    ObjFunction* function = endCompiler(&compiler);

    // Return function for no compile error. 
    return parser.hadError ? NULL : function;
}

void markCompilerRoots(VM* vm) {
    Compiler* compiler = vm->compiler;
    while (compiler != NULL) {
        markObject(vm, (Obj*)compiler->function);
        compiler = compiler->enclosing;
    }
}
