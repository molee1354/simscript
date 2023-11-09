#ifndef simscript_scanner_h
#define simscript_scanner_h


/**
 * @brief Supported token types in the language
 *
 */
typedef enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, TOKEN_MOD,
    TOKEN_COLON,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Operate and Reassign
    TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS, TOKEN_PLUS_EQUALS,
    TOKEN_MINUS_EQUALS, TOKEN_SLASH_EQUALS, TOKEN_STAR_EQUALS,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_LOCAL, TOKEN_NULL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_CONST, TOKEN_WHILE, TOKEN_INHERIT,
    TOKEN_MODULE, TOKEN_AS, TOKEN_BREAK, TOKEN_CONTINUE, TOKEN_USING,

    TOKEN_ERROR, TOKEN_EOF
} Tokentype;

/**
 * @brief Token struct to hold the token defintion
 *
 */
typedef struct {
    Tokentype type;
    const char* start;
    int length;
    int line;
} Token;

/**
 * @brief Initialize the Scanner struct
 * @param source An array of characters collected from the input stream.
 *
 */
void initScanner(const char* source);

/**
 * @brief Method to scan and return the current token.
 *
 */
Token scanToken();

#endif
