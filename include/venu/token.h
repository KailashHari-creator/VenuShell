#ifndef VENU_TOKEN_H
#define VENU_TOKEN_H

#include <stddef.h>

enum venu_token_type {
    /*
     * Textual tokens.
     */
    VENU_TOKEN_WORD,
    VENU_TOKEN_STRING,
    VENU_TOKEN_LITERAL_STRING,
    VENU_TOKEN_NUMBER,

    /*
     * Layout.
     */
    VENU_TOKEN_NEWLINE,
    VENU_TOKEN_END,

    /*
     * Punctuation.
     */
    VENU_TOKEN_LEFT_PARENTHESIS,
    VENU_TOKEN_RIGHT_PARENTHESIS,
    VENU_TOKEN_LEFT_BRACE,
    VENU_TOKEN_RIGHT_BRACE,
    VENU_TOKEN_COMMA,
    VENU_TOKEN_SEMICOLON,
    VENU_TOKEN_DOLLAR,

    /*
     * Arithmetic.
     */
    VENU_TOKEN_PLUS,
    VENU_TOKEN_MINUS,
    VENU_TOKEN_STAR,
    VENU_TOKEN_SLASH,
    VENU_TOKEN_PERCENT,

    /*
     * Assignment and comparison.
     */
    VENU_TOKEN_ASSIGN,
    VENU_TOKEN_EQUAL,
    VENU_TOKEN_NOT,
    VENU_TOKEN_NOT_EQUAL,
    VENU_TOKEN_LESS,
    VENU_TOKEN_LESS_EQUAL,
    VENU_TOKEN_GREATER,
    VENU_TOKEN_GREATER_EQUAL,

    /*
     * Command control.
     */
    VENU_TOKEN_PIPE,
    VENU_TOKEN_AND,
    VENU_TOKEN_OR,
    VENU_TOKEN_AMPERSAND,
    VENU_TOKEN_APPEND,

    VENU_TOKEN_ERROR
};

struct venu_token {
    enum venu_token_type type;

    const char *start;
    size_t length;

    size_t line;
    size_t column;
};

const char *venu_token_type_name(
    enum venu_token_type type
);

#endif