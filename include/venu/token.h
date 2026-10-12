#ifndef VENU_TOKEN_H
#define VENU_TOKEN_H

#include <stddef.h>

enum venu_token_type 
{
    VENU_TOKEN_WORD,
    VENU_TOKEN_STRING,
    VENU_TOKEN_NUMBER,
    VENU_TOKEN_NEWLINE,
    VENU_TOKEN_END,
    VENU_TOKEN_ERROR
};

struct venu_token 
{
    enum venu_token_type type;

    const char *start;
    size_t length;

    size_t line;
    size_t column;
};

const char *venu_token_type_name (enum venu_token_type type);

#endif