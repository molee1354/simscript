#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;


void initScanner(const char* source) {
    // initialize both scanner pointers to the first character
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

/**
 * @brief Method to determine if the current character is an alphabet
 * @param c Current character
 *
 * @return bool True if the current character is a alphabet
 */
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

/**
 * @brief Method to determine if the current character is a digit
 * @param c Current character
 *
 * @return bool True if the current character is a digit
 */
static bool isDigit(char c) {
    return c>='0' && c<='9';
}

/**
 * @brief Method to determine if the scanner reached the end of the file
 *
 * @return bool True if the current scanner position points to \0
 */
static bool isAtEnd() {
    return *scanner.current == '\0';
}

/** 
 * @brief Method to read the next character and advance scanner.current.
 * 
 * @return char The current character the scanner points to
 */
static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

/**
 * @brief Method to peek at the current character and not advance.
 *
 * @return char The character at the current scanner position
 */
static char peek() {
    return *scanner.current;
}

/**
 * @brief Method to peek at the next character and not advance.
 *
 * @return char The character after the current character
 */
static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

/**
 * @brief Method to match the current character to an expected character
 * and advance scanner.current.
 *
 * return bool True and advance if the current character matches the
 * expected character
 */
static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

/**
 * @brief Token constructor
 * @param type The type of the token to be constructed
 *
 * @return Token new token
 */
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

/**
 * @brief Error token generator. Sets return token to TOKEN_ERROR
 * @param message The error message that goes with the error token
 *
 * @return Token new error token
 */
static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

/**
 * @brief Method to skip (advance()) whitespace
 *
 */
static void skipWhiteSpace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            // for a newline, increment the line number
            case '\n':
                scanner.line++;
                advance();
                break;

            // Comments signals ignore the line until the next newline
            case '/':
                if (peekNext() == '/') {
                    while ( peek() != '\n' && !isAtEnd() ) advance();
                } else {
                    return;
                }
                break;
            case '#':
                if (scanner.line < 2 && peekNext() == '!') { 
                    while( peek() != '\n' && !isAtEnd() ) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

/**
 * @brief Method to check if the current identifier is a keyword.
 * @param start Index at which the identifier starts
 * @param length The length of the identifier
 * @param rest The remainder of the keyword after the first letter
 * @param type Type of token to return based on keyword
 *
 * @return TokenType The corresponding TokenType for the identifier token.
 * Returns TOKEN_IDENTIFIER if no matches occur.
 */
static TokenType checkKeyword(int start, int length, 
        const char* rest, TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Method to return the correct identifier type
 *
 * @return TokenType identifier token type
 */
static TokenType identifierType() {

    switch (scanner.start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            if (scanner.current-scanner.start > 1) {
              switch (scanner.start[1]) {
                  case 'l': return checkKeyword(2,3, "ass", TOKEN_CLASS);
                  case 'o': return checkKeyword(2,3, "nst", TOKEN_CONST);
              }
            }
            break;
        case 'e':
            if (scanner.current-scanner.start > 1) {
              switch (scanner.start[1]) {
                  case 'l': return checkKeyword(2,2, "se", TOKEN_ELSE);
                  case 'x': return checkKeyword(2,5, "tends", TOKEN_INHERIT);
              }
            }
            break;
        case 'f':
            if (scanner.current-scanner.start > 1) {
              switch (scanner.start[1]) {
                  case 'a': return checkKeyword(2,3, "lse", TOKEN_FALSE);
                  case 'o': return checkKeyword(2,1, "r", TOKEN_FOR);
                  case 'u': return checkKeyword(2,6, "nction", TOKEN_FUN);
              }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'l': return checkKeyword(1, 2, "et", TOKEN_LET);
        case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current-scanner.start > 1) {
              switch (scanner.start[1]) {
                  case 'h': return checkKeyword(2,2, "is", TOKEN_THIS);
                  case 'r': return checkKeyword(2,2, "ue", TOKEN_TRUE);
              }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Method to generate the identifier token
 *
 * @return Token Token of type TOKEN_IDENTIFIER.
 */
static Token identifier() {
    while ( isAlpha(peek()) || isDigit(peek()) ) advance();
    return makeToken(identifierType());
}

/**
 * @brief Method to generate the number token
 *
 * @return Token Token of type TOKEN_NUMBER.
 */
static Token number() {
    while ( isDigit(peek()) ) advance();

    // Looking for fractional parts
    if ( peek() == '.' && isDigit(peekNext()) ) {
        // consume the '.'
        advance();

        while ( isDigit(peek()) ) advance();
    }
    return makeToken(TOKEN_NUMBER);
}

/**
 * @brief Method to generate the string token
 *
 * @return Token Token of type TOKEN_STRING. Returns an error token if the
 * string is unterminated.
 */
static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        // multiline string support
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // Reaches here when closing quote is encountered
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    // skipping whitespace before scanning
    skipWhiteSpace();

    scanner.start = scanner.current;
    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        // recognizing single character tokens
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case '.': return makeToken(TOKEN_DOT);
        case ',': return makeToken(TOKEN_COMMA);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '%': return makeToken(TOKEN_MOD);

        // recognizing double character tokens
        case '!':
          return makeToken(
              match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
          return makeToken(
              match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
          return makeToken(
              match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
          return makeToken(
              match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // literal tokens
        case '"': return string();
    }

    return errorToken("Unexpected Character.");
}
