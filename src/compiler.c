#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


// Parser parser;

// global compilers bad for multithreading
// Compiler* current = NULL;

// Compiler for class scope
// ClassCompiler* currentClass = NULL;

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
    fprintf(stderr, "COMPILER ERROR:\n[line %d] Error", token->line);

    if (token->type==TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // pass
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
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
        parser->current = scanToken();
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
static void consume(Compiler* compiler, TokenType type, const char* message) {
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
static bool check(Compiler* compiler, TokenType type) {
    return compiler->parser->current.type == type;
}

/**
 * @brief Method to check the current type going through the parser
 *
 * @param type The type to match
 * @return True if the types match
 */
static bool match(Compiler* compiler, TokenType type) {
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
    writeChunk(currentChunk(compiler), byte, compiler->parser->previous.line);
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
    int constant = addConstant(currentChunk(compiler), value);

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
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();

    // storing the function's name (if not top-level/script)
    if (type != TYPE_SCRIPT) {
        compiler->function->name = copyString(parser->previous.start,
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
    compiler = compiler->enclosing;
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
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Compiler* compiler, Precedence precedence);

static uint8_t argumentList(Compiler* compiler);

/**
 * @brief Method to write the constant name as a string to the table
 *
 * @param name Name of the token
 * @return uint8_t index of the constant in the program
 */
static uint8_t identifierConstant(Compiler* compiler, Token* name) {
    return makeConstant(compiler, OBJ_VAL(copyString(name->start,
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
static ResolvedLocal resolveLocal(Compiler* compiler, Token* name) {
    ResolvedLocal out = {-1, false, false};
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
static int resolveUpvalue(Compiler* compiler, Token* name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolveLocal(compiler->enclosing, name).depth;
    bool isScoped = resolveLocal(compiler->enclosing, name).isScoped;
    if (isScoped) return -1;
    if (local != -1) { // if local var is found
        // for resolving an identifier, mark it as "captured"
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    /* recursive use of resolveUpvalue to capture from its immediately
     * surrounding function (arg to addUpvalue is false). 
     * This is for "not top-level" 
     */
    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    }
    return -1;
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
static void binary(Compiler* compiler, bool canAssign __attribute__((unused)) ) {
    TokenType operatorType = compiler->parser->previous.type;
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

        // unreachable
        default: return;
    }
}

/**
 * @brief Method to parse function calls
 *
 * @param canAssign True if assignable
 */
static void call(Compiler* compiler, bool canAssign __attribute__((unused))) {
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

    } else if (match(compiler, TOKEN_PLUS_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        expression(compiler);
        emitByte(compiler, OP_ADD);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (match(compiler, TOKEN_MINUS_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        expression(compiler);
        emitByte(compiler, OP_SUBTRACT);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (match(compiler, TOKEN_STAR_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        expression(compiler);
        emitByte(compiler, OP_MULTIPLY);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (match(compiler, TOKEN_SLASH_EQUALS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        expression(compiler);
        emitByte(compiler, OP_DIVIDE);
        emitBytes(compiler, OP_SET_PROPERTY, name);

    } else if (match(compiler, TOKEN_PLUS_PLUS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        emitByte(compiler, OP_INCREMENT);
        emitBytes(compiler, OP_SET_PROPERTY, name);
    } else if (match(compiler, TOKEN_MINUS_MINUS)) {
        emitBytes(compiler, OP_GET_PROPERTY, name);
        expression(compiler);
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
static void literal(Compiler* compiler, bool canAssign __attribute__((unused))) {
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
static void grouping(bool canAssign __attribute__((unused))) {
    expression(); // takes care of generating bytecode inside the parenthesis
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * @brief Method to convert a parsed string to a number
 *
 */
static void number(bool canAssign __attribute__((unused))) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

/**
 * @brief Method to handle the 'and' logical operation
 *
 * @param canAssign Variable to check if the value can be assigned
 */
static void and_(bool canAssign __attribute__((unused))) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    patchJump(endJump);
}

/**
 * @brief Method to handle the 'or' logical operation
 *
 * @param canAssign Variable to check if the value can be assigned
 */
static void or_(bool canAssign __attribute__((unused))) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

/**
 * @brief Method to convert a parsed string into string value
 *
 */
static void string(bool canAssign __attribute__((unused))) {
    emitConstant( OBJ_VAL(copyString(parser.previous.start + 1,
                                     parser.previous.length -2)) );
}

/**
 * @brief TODO add comment here
 *
 * @param name 
 */
static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    ResolvedLocal resLoc = resolveLocal(current, &name);
    int arg = resLoc.depth;
    bool isConst = resLoc.isConst;
    bool isScoped = resLoc.isScoped;

    if (arg != -1) { // -1 for global state
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ( !isScoped && (arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        expression();
        emitBytes(setOp, (uint8_t)arg);

    } else if (canAssign && match(TOKEN_PLUS_PLUS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        emitByte(OP_INCREMENT);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_MINUS_MINUS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        emitByte(OP_DECREMENT);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_PLUS_EQUALS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        expression();
        emitByte(OP_ADD);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_MINUS_EQUALS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        expression();
        emitByte(OP_SUBTRACT);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_STAR_EQUALS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        expression();
        emitByte(OP_MULTIPLY);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_SLASH_EQUALS)) {
        if (isConst) {
            error("Cannot reassign values to constants.");
        }
        namedVariable(name, false);
        expression();
        emitByte(OP_DIVIDE);
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

/**
 * @brief Method to parse variables
 *
 */
static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
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

static void super_(bool canAssign __attribute__((unused))) {
    // limiting the use of super
    if (currentClass == NULL) {
        error("Can't use 'super' outside of a class.");
    } else if (!currentClass->hasSuperClass) {
        error("Can't use 'super' in a class with no parent.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(&parser.previous);

    namedVariable(syntheticToken("this"), false);

    // Differentiating "invocations" and "gets"
    if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_SUPER_INVOKE, name);
        emitByte(argCount);
    } else {
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_GET_SUPER, name);
    }
}

/**
 * @brief Method to parse the 'this' keyword. Treated as a lexically scoped
 * local variable whose value gets initialized. 
 *
 * @param canAssign 
 */
static void this_(bool canAssign __attribute__((unused))) {
    if (currentClass == NULL) {
        error("Using 'this' out of a classdef context.");
        return;
    }
    variable(false);
}

/**
 * @brief Method to deal with the unary minus
 *
 */
static void unary(bool canAssign __attribute__((unused))) {
    TokenType operatorType = parser.previous.type;

    // compiling the operand
    parsePrecedence(PREC_UNARY);

    // emit operator instruction
    switch (operatorType) {
        case TOKEN_BANG:  emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; // unreachable
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
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     dot,   PREC_CALL},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
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

    [TOKEN_PLUS_PLUS]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_PLUS_EQUALS]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS_MINUS]   = {NULL,     NULL,   PREC_NONE},
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
    [TOKEN_IMPORT]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
    [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LET]           = {NULL,     NULL,   PREC_NONE},
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
static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;

    if (prefixRule==NULL) {
        error("Expect expression.");
        return;
    }

    // checking if assignment can happen
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
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
static uint8_t parseVariable(const char* errorMessage,
                             bool isConst, bool isScoped) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable(isConst, isScoped);
    if (current->scopeDepth > 0) return 0; // exit the function if local scope

    return identifierConstant(&parser.previous);
}

/**
 * @brief Method to mark a variable as initialized.
 */
static void markInitialized() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

/**
 * @brief Method to output the bytecode instruction that defines the new
 * variable and stores its initial value
 *
 * @param global Variable index
 */
static void defineVariable(Compiler* compiler, uint8_t global) {
    // don't define var if in local scope
    if (compiler->scopeDepth > 0) {
        markInitialized();
        return;
    }
    emitBytes(compiler, OP_DEFINE_GLOBAL, global);
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
            if (argCount >= 255) error("Can't have more than 255 argumants.");
            argCount++;
        } while (match(compiler, TOKEN_COMMA));
    }
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after function arguments.");
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
static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

/**
 * @brief Method to parse expressions
 *
 */
static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

/**
 * @brief Method to compile the function. A separate compiler is created for
 * each function being compiled.
 *
 * @param type The type of function to compile
 */
static void beginFunction(Compiler* compiler, Compiler* funcCompiler, FunctionType type) {
    initCompiler(compiler->parser, compiler, funcCompiler, type);
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

    endCompiler(&fnCompiler);
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
    Compiler funcCompiler;
    beginFunction(compiler, &funcCompiler, type);
    endCompiler(&funcCompiler);
    emitBytes(compiler, OP_METHOD, constant);
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
    classCompiler.hasSuperClass = false;
    classCompiler.enclosing = currentClass; // at toplevel set to global class
    currentClass = &classCompiler; // subsequent nested levels point to new
                                   // class compiler

    // class inheritance
    if (match(TOKEN_INHERIT)) {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(false);

        if (identifiersEqual(&className, &parser.previous)) {
            error("Invalid inheritance from self.");
        }
        // add a superclass local variable
        beginScope(); // each class has a local scope store "super"

        // default behavior "false, false" : re-assignable, not-scoped.
        addLocal( syntheticToken("super"), false, false);
        defineVariable(0);

        namedVariable(className, false);
        emitByte(OP_INHERIT);
        classCompiler.hasSuperClass = true;
    }

    // class definition
    namedVariable(className, false);
    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        method();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' before class body.");
    emitByte(OP_POP);

    if (classCompiler.hasSuperClass) {
        endScope();
    }

    // set current class to it's enclosing class after it's done declaring.
    // NULL if at global scope.
    currentClass = currentClass->enclosing;
}

/**
 * @brief Method to parse function declarations as a first class value.
 */
static void funDeclaration() {
    uint8_t global = parseVariable("Expect function name.", false, false);
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

/**
 * @brief A method to handle variable declarations.
 */
static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.", false, false);

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NULL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(global);
}

static void constVarDec() {
    uint8_t global = parseVariable("Expect variable name.", true, false);

    if (!match(TOKEN_EQUAL)) {
        error("Constant declarations must be followed by a value assignment.");
    } else {
        expression();
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after constant declaration");

    defineVariable(global);
}

static void letDeclaration() {
    uint8_t global = parseVariable("Expect variable name.", false, true);

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NULL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after constant declaration");

    defineVariable(global);
}

static void constLetDec() {
    uint8_t global = parseVariable("Expect variable name.", true, true);

    if (!match(TOKEN_EQUAL)) {
        error("Constant declarations must be followed by a value assignment.");
    } else {
        expression();
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after constant declaration");

    defineVariable(global);
}

/**
 * @brief Method to compile an expression followed by a semicolon.
 *
 */
static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

/**
 * @brief Method to get the argument count of a given OpCode
 *
 * @param code 
 * @param constants 
 * @param ip 
 * @return 
 */
static int getArgCount(uint8_t *code, const ValueArray constants, int ip) {
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
        case OP_NOT:
        case OP_NEGATE:
        case OP_CLOSE_UPVALUE:
        case OP_RETURN:
        case OP_BREAK:
            return 0;

        case OP_CONSTANT:
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
        case OP_GET_GLOBAL:
        case OP_GET_UPVALUE:
        case OP_SET_UPVALUE:
        case OP_GET_SUPER:
        case OP_METHOD:
        case OP_IMPORT:
            return 1;

        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_LOOP:
        case OP_CLASS:
        case OP_CALL:
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
static void endLoop() {
    if (current->loop->end != -1) {
        patchJump(current->loop->end);
        emitByte(OP_POP);
    }

    int i = current->loop->body;
    while (i < current->function->chunk.count) {
        if (current->function->chunk.code[i] == OP_BREAK) {
            current->function->chunk.code[i] = OP_JUMP;
            patchJump(i+1)j;
            i+=3;
        } else {
            i += 1 + getArgCount(current->function->chunk.code,
                                 current->function->chunk.constants, i);
        }
    }
    current->loop = current->loop->enclosing;
}
/**
 * @brief Method to parse through for statements
 */
static void forStatement() {
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // init clause
    if(match(TOKEN_SEMICOLON)) {
        // no init
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    Loop loop;
    loop.start = currentChunk()->count;
    loop.scopeDepth = current->scopeDepth;
    loop.enclosing = current->loop;
    current->loop = &loop;

    // condition clause
    current->loop->end = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // exit loop if condition is false
        current->loop->end = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    // increment clause
    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(current->loop->start);
        current->loop->start = incrementStart;
        patchJump(bodyJump);
    }
    
    current->loop->body = current->function->chunk.count;
    statement();
    emitLoop(current->loop->start);

    endLoop();
    endScope();
}

static void breakStatement() {
    if (current->loop == NULL) {
        error("'break' statements can only be used in a loop.");
        return;
    }

    consume(TOKEN_SEMICOLON, "Expected ';' after 'break'.");

    // getting rid of locals in the loop scope
    for (int i = current->localCount - 1;
         i >= 0 && current->locals[i].depth > current->loop->scopeDepth;
         i--) {
        if (current->locals[i].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
    }
    emitJump(OP_BREAK);
}

static void continueStatement() {
    if (current->loop == NULL) {
        error("'continue' statements can only be used in a loop.");
        return;
    }

    consume(TOKEN_SEMICOLON, "Expected ';' after 'continue'.");

    // getting rid of locals in the loop scope
    for (int i = current->localCount - 1;
         i >= 0 && current->locals[i].depth > current->loop->scopeDepth;
         i--) {
        if (current->locals[i].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
    }

    // go back to the current loop start
    emitLoop(current->loop->start);
}

/**
 * @brief Method to parse through the if statement
 */
static void ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'if'.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

/**
 * @brief Method to handle print statements
 */
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after 'echo' argument.");
    emitByte(OP_PRINT);
}

/**
 * @brief Method to handle import statements
 */
static void importStatement() {
    consume(TOKEN_STRING, "Expect filepath after import");
    emitConstant(OBJ_VAL(
        copyString(parser.previous.start + 1, parser.previous.length - 2)));
    // expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after import path.");
    emitByte(OP_IMPORT);
}

/**
 * @brief Method to handle return statements
 */
static void returnStatement() {
    if (current->type == TYPE_SCRIPT) {
        error("Cannot return from top-level code.");
    }
    if (match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        if (current->type == TYPE_INITIALIZER) {
            error("Invalid attempt to return from an initializer.");
        }
        expression();
        consume(TOKEN_SEMICOLON, "Expected ';' after return statement.");
        emitByte(OP_RETURN);
    }
}

/**
 * @brief Method to handle while statements
 */
static void whileStatement() {
    Loop loop;
    loop.start = currentChunk()->count;
    loop.scopeDepth = current->scopeDepth;
    loop.enclosing = current->loop;
    current->loop = &loop;

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'while'.");

    current->loop->end = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    current->loop->body = current->function->chunk.count;
    statement();

    emitLoop(loop.start);
    endLoop();
}

/**
 * @brief Method to synchronize panic mode. Allows the compiler to exit
 * panic mode when it reaches a synchronization point.
 */
static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_CONST:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                ; // do nothing
        }
        advance();
    }
}

/**
 * @brief Method to compile a single declaration
 *
 */
static void declaration() {
    if (match(TOKEN_CLASS)) {
        classDeclaration();
    } else if (match(TOKEN_FUN)) {
        funDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else if (match(TOKEN_LET)) {
        letDeclaration();
    } else if (match(TOKEN_CONST)) {
        if (match(TOKEN_VAR)) {
            constVarDec();
        } else if (match(TOKEN_LET)) {
            constLetDec();
        } else {
            error("Expected variable declaration after 'const'.");
        }
    } else {
        statement();
    }

    // if parser gets into panic mode, we synchronize
    if (parser.panicMode) synchronize();
}

/**
 * @brief Method to match statements according to their tokens
 * 
 */
static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_IMPORT)) {
        importStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_BREAK)) {
        breakStatement();
    } else if (match(TOKEN_CONTINUE)) {
        continueStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

ObjFunction* compile(const char *source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    // initializing error state
    parser.hadError = false;
    parser.panicMode = false;

    advance();
    while (!match(TOKEN_EOF)) {
        declaration();
    }
    ObjFunction* function = endCompiler();

    // Return function for no compile error. 
    return parser.hadError ? NULL : function;
}

void markCompilerRoots() {
    Compiler* compiler = current;
    while (compiler != NULL) {
        markObject((Obj*)compiler->function); 
        compiler = compiler->enclosing;
    }
}
