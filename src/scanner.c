#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"


void initScanner(Scanner* scanner, const char* source) {
    // initialize both scanner pointers to the first character
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
    scanner->rawString = false;
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
static bool isAtEnd(Scanner* scanner) {
    return *scanner->current == '\0';
}

/** 
 * @brief Method to read the next character and advance scanner.current.
 * 
 * @return char The current character the scanner points to
 */
static char advance(Scanner* scanner) {
    scanner->current++;
    return scanner->current[-1];
}

/**
 * @brief Method to peek at the current character and not advance.
 *
 * @return char The character at the current scanner position
 */
static char peek(Scanner* scanner) {
    return *scanner->current;
}

/**
 * @brief Method to peek at the next character and not advance.
 *
 * @return char The character after the current character
 */
static char peekNext(Scanner* scanner) {
    if (isAtEnd(scanner)) return '\0';
    return scanner->current[1];
}

/**
 * @brief Method to match the current character to an expected character
 * and advance scanner.current.
 *
 * return bool True and advance if the current character matches the
 * expected character
 */
static bool match(Scanner* scanner, char expected) {
    if (isAtEnd(scanner)) return false;
    if (*scanner->current != expected) return false;
    scanner->current++;
    return true;
}

/**
 * @brief Token constructor
 * @param type The type of the token to be constructed
 *
 * @return Token new token
 */
static Token makeToken(Scanner* scanner, Tokentype type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

/**
 * @brief Error token generator. Sets return token to TOKEN_ERROR
 * @param message The error message that goes with the error token
 *
 * @return Token new error token
 */
static Token errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    return token;
}

/**
 * @brief Method to skip (advance()) whitespace
 *
 */
static void skipWhiteSpace(Scanner* scanner) {
    for (;;) {
        char c = peek(scanner);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(scanner);
                break;

            // for a newline, increment the line number
            case '\n':
                scanner->line++;
                advance(scanner);
                break;

            // Comments signals ignore the line until the next newline
            case '/':
                if (peekNext(scanner) == '/') {
                    while ( peek(scanner) != '\n' && !isAtEnd(scanner) ) advance(scanner);
                } else if (peekNext(scanner) == '*') {
                    advance(scanner);
                    advance(scanner);
                    while ( !isAtEnd(scanner) ) {
                        if (peek(scanner) == '*' && peekNext(scanner) == '/') {
                            advance(scanner);
                            advance(scanner);
                            break;
                        }
                        advance(scanner);
                    }
                } else {
                    return;
                }
                break;
            case '#':
                if (scanner->line < 2 && peekNext(scanner) == '!') { 
                    while( peek(scanner) != '\n' && !isAtEnd(scanner) ) advance(scanner);
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
 * @return Tokentype The corresponding Tokentype for the identifier token.
 * Returns TOKEN_IDENTIFIER if no matches occur.
 */
static Tokentype checkKeyword(Scanner* scanner, int start, int length,
        const char* rest, Tokentype type) {
    if (scanner->current - scanner->start == start + length &&
        memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Method to return the correct identifier type
 *
 * @return Tokentype identifier token type
 */
static Tokentype identifierType(Scanner* scanner) {

    switch (scanner->start[0]) {
        case 'a':
            if (scanner->current-scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 's': return checkKeyword(scanner, 2,0, "", TOKEN_AS);
                    case 'n': return checkKeyword(scanner, 2,1, "d", TOKEN_AND);
                }
            }
            break;
        case 'b': return checkKeyword(scanner, 1, 4, "reak", TOKEN_BREAK);
        case 'c':
            if (scanner->current-scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'l': return checkKeyword(scanner, 2,3, "ass", TOKEN_CLASS);
                    case 'o':
                        if (scanner->current-scanner->start > 3) {
                            switch (scanner->start[3]) {
                                case 's':
                                    return checkKeyword(scanner, 4,1, "t", TOKEN_CONST);
                                case 't':
                                    return checkKeyword(scanner, 4,4, "inue", TOKEN_CONTINUE);
                            }
                        }
                }
            }
            break;
        case 'e':
            if (scanner->current-scanner->start > 1) {
              switch (scanner->start[1]) {
                  case 'l': return checkKeyword(scanner, 2,2, "se", TOKEN_ELSE);
                  case 'x': return checkKeyword(scanner, 2,5, "tends", TOKEN_INHERIT);
                  case 'c': return checkKeyword(scanner, 2,2, "ho", TOKEN_PRINT);
              }
            }
            break;
        case 'f':
            if (scanner->current-scanner->start > 1) {
              switch (scanner->start[1]) {
                  case 'a': return checkKeyword(scanner, 2,3, "lse", TOKEN_FALSE);
                  case 'o': return checkKeyword(scanner, 2,1, "r", TOKEN_FOR);
                  case 'u': return checkKeyword(scanner, 2,6, "nction", TOKEN_FUN);
              }
            }
            break;
        case 'i': return checkKeyword(scanner, 1, 1, "f", TOKEN_IF);
        case 'l': return checkKeyword(scanner, 1, 4, "ocal", TOKEN_LOCAL);
        case 'm': return checkKeyword(scanner, 1, 5, "odule", TOKEN_MODULE);
        case 'n': return checkKeyword(scanner, 1, 3, "ull", TOKEN_NULL);
        case 'o': return checkKeyword(scanner, 1, 1, "r", TOKEN_OR);
        case 'r':
                  if (scanner->current-scanner->start > 1) {
                      switch(scanner->start[1]) {
                          case 'e': return checkKeyword(scanner, 2, 4, "turn", TOKEN_RETURN);
                      }
                  } else {
                      if (scanner->start[1] == '"' || scanner->start[1] == '\'') {
                          scanner->rawString = true;
                          return TOKEN_RSTRING;
                      }
                  }
                  break;
        case 's': return checkKeyword(scanner, 1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner->current-scanner->start > 1) {
              switch (scanner->start[1]) {
                  case 'h': return checkKeyword(scanner, 2,2, "is", TOKEN_THIS);
                  case 'r': return checkKeyword(scanner, 2,2, "ue", TOKEN_TRUE);
              }
            }
            break;
        case 'u': return checkKeyword(scanner, 1, 4, "sing", TOKEN_USING);
        case 'v': return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Method to generate the identifier token
 *
 * @return Token Token of type TOKEN_IDENTIFIER.
 */
static Token identifier(Scanner* scanner) {
    while ( isAlpha(peek(scanner)) || isDigit(peek(scanner)) ) advance(scanner);
    return makeToken(scanner, identifierType(scanner));
}

/**
 * @brief Method to generate the number token
 *
 * @return Token Token of type TOKEN_NUMBER.
 */
static Token number(Scanner* scanner) {
    while ( isDigit(peek(scanner)) ) advance(scanner);

    // Looking for fractional parts
    if ( peek(scanner) == '.' && isDigit(peekNext(scanner)) ) {
        // consume the '.'
        advance(scanner);

        while ( isDigit(peek(scanner)) ) advance(scanner);
    }
    return makeToken(scanner, TOKEN_NUMBER);
}

/**
 * @brief Method to generate the string token
 *
 * @return Token Token of type TOKEN_STRING. Returns an error token if the
 * string is unterminated.
 */
static Token string(Scanner* scanner, char termChar) {
    while (peek(scanner) != termChar && !isAtEnd(scanner)) {
        // multiline string support
        if (peek(scanner) == '\n') {
            scanner->line++;
        } else if (peek(scanner) == '\\' && !scanner->rawString) {
            scanner->current++;
        }
        advance(scanner);
    }

    if (isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");

    // Reaches here when closing quote is encountered
    advance(scanner);
    scanner->rawString = false;
    return makeToken(scanner, TOKEN_STRING);
}

Token scanToken(Scanner* scanner) {
    // skipping whitespace before scanning
    skipWhiteSpace(scanner);

    scanner->start = scanner->current;
    if (isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);

    char c = advance(scanner);
    if (isAlpha(c)) return identifier(scanner);
    if (isDigit(c)) return number(scanner);

    switch (c) {
        // recognizing single character tokens
        case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
        case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
        case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
        case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
        case '[': return makeToken(scanner, TOKEN_LEFT_BRACKET);
        case ']': return makeToken(scanner, TOKEN_RIGHT_BRACKET);
        case ';': return makeToken(scanner, TOKEN_SEMICOLON);
        case ':': return makeToken(scanner, TOKEN_COLON);
        case '.': return makeToken(scanner, TOKEN_DOT);
        case ',': return makeToken(scanner, TOKEN_COMMA);
        case '-': {
            if (match(scanner, '-')) {
                return makeToken(scanner, TOKEN_MINUS_MINUS);
            } else if (match(scanner, '=')) {
                return makeToken(scanner, TOKEN_MINUS_EQUALS);
            } else {
                return makeToken(scanner, TOKEN_MINUS);
            }
        }
        case '+': {
            if (match(scanner, '+')) {
                return makeToken(scanner, TOKEN_PLUS_PLUS);
            } else if (match(scanner, '=')) {
                return makeToken(scanner, TOKEN_PLUS_EQUALS);
            } else {
                return makeToken(scanner, TOKEN_PLUS);
            }
        }
        case '/': {
            if (match(scanner, '=')) {
                return makeToken(scanner, TOKEN_SLASH_EQUALS);
            } else {
                return makeToken(scanner, TOKEN_SLASH);
            }
        }
        case '*': {
            if (match(scanner, '=')) {
                return makeToken(scanner, TOKEN_STAR_EQUALS);
            } else {
                return makeToken(scanner, TOKEN_STAR);
            }
        }
        case '%': return makeToken(scanner, TOKEN_MOD);

        // recognizing double character tokens
        case '!':
          return makeToken(scanner, 
              match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
          return makeToken(scanner, 
              match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
          return makeToken(scanner, 
              match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
          return makeToken(scanner, 
              match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // literal tokens
        case '\'': return string(scanner, '\'');
        case '"': return string(scanner, '"');
    }

    return errorToken(scanner, "Unexpected Character.");
}
