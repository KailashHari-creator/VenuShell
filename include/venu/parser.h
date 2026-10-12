#ifndef VENU_PARSER_H
#define VENU_PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "venu/arena.h"
#include "venu/ast.h"
#include "venu/lexer.h"

/*
 * Legacy command parser.
 *
 * Keep this temporarily so the existing shell continues working.
 */
#define VENU_COMMAND_NAME_CAPACITY 64
#define VENU_MAX_ARGUMENTS 16
#define VENU_ARGUMENT_CAPACITY 128
#define VENU_PARSE_ERROR_CAPACITY 128

struct venu_command {
    char name[VENU_COMMAND_NAME_CAPACITY];

    size_t argument_count;

    char arguments[
        VENU_MAX_ARGUMENTS
    ][VENU_ARGUMENT_CAPACITY];
};

struct venu_parse_result {
    bool success;
    bool empty;

    char error[VENU_PARSE_ERROR_CAPACITY];
};

struct venu_parse_result venu_parse_command(
    struct venu_lexer *lexer,
    struct venu_command *command
);

/*
 * New AST parser.
 */
struct venu_ast_parse_result {
    bool success;

    struct venu_ast_program *program;

    size_t error_line;
    size_t error_column;

    char error[VENU_PARSE_ERROR_CAPACITY];
};

struct venu_ast_parse_result venu_parse_program(
    struct venu_lexer *lexer,
    struct venu_arena *arena
);

#endif