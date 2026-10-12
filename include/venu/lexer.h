#ifndef VENU_LEXER_H
#define VENU_LEXER_H

#include <stddef.h>

#include "venu/token.h"

struct venu_lexer 
{
    const char *source;
    size_t length;
    size_t position;

    size_t line;
    size_t column;
};

void venu_lexer_initialize (struct venu_lexer *lexer, const char *source, size_t length);
struct venu_token venu_lexer_next (struct venu_lexer *lexer);

#endif