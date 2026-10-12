#ifndef VENU_AST_H
#define VENU_AST_H

#include <stddef.h>

#include "venu/value.h"

struct venu_source_location {
    size_t line;
    size_t column;
};

struct venu_ast_text {
    /*
     * This refers to characters in the original source buffer.
     * The source must remain alive while the AST is being used.
     */
    const char *data;
    size_t length;
};

enum venu_ast_unary_operator {
    VENU_AST_UNARY_NEGATE,
    VENU_AST_UNARY_NOT
};

enum venu_ast_binary_operator {
    VENU_AST_BINARY_ADD,
    VENU_AST_BINARY_SUBTRACT,
    VENU_AST_BINARY_MULTIPLY,
    VENU_AST_BINARY_DIVIDE,
    VENU_AST_BINARY_REMAINDER,

    VENU_AST_BINARY_EQUAL,
    VENU_AST_BINARY_NOT_EQUAL,
    VENU_AST_BINARY_LESS,
    VENU_AST_BINARY_LESS_EQUAL,
    VENU_AST_BINARY_GREATER,
    VENU_AST_BINARY_GREATER_EQUAL,

    VENU_AST_BINARY_AND,
    VENU_AST_BINARY_OR
};

enum venu_ast_expression_type {
    VENU_AST_EXPRESSION_LITERAL,
    VENU_AST_EXPRESSION_VARIABLE,
    VENU_AST_EXPRESSION_UNARY,
    VENU_AST_EXPRESSION_BINARY
};

struct venu_ast_expression;

struct venu_ast_unary_expression {
    enum venu_ast_unary_operator operator;
    struct venu_ast_expression *operand;
};

struct venu_ast_binary_expression {
    enum venu_ast_binary_operator operator;

    struct venu_ast_expression *left;
    struct venu_ast_expression *right;
};

struct venu_ast_expression {
    enum venu_ast_expression_type type;
    struct venu_source_location location;

    union {
        struct venu_value literal;
        struct venu_ast_text variable;
        struct venu_ast_unary_expression unary;
        struct venu_ast_binary_expression binary;
    } as;
};

struct venu_ast_argument {
    struct venu_ast_expression *expression;
    struct venu_ast_argument *next;
};

struct venu_ast_command {
    struct venu_ast_text name;

    struct venu_ast_argument *first_argument;
    struct venu_ast_argument *last_argument;

    size_t argument_count;
};

struct venu_ast_variable_declaration {
    struct venu_ast_text name;
    struct venu_ast_expression *initializer;
};

struct venu_ast_assignment {
    struct venu_ast_text name;
    struct venu_ast_expression *value;
};

enum venu_ast_statement_type {
    VENU_AST_STATEMENT_COMMAND,
    VENU_AST_STATEMENT_VARIABLE_DECLARATION,
    VENU_AST_STATEMENT_ASSIGNMENT,
    VENU_AST_STATEMENT_EXPRESSION
};

struct venu_ast_statement {
    enum venu_ast_statement_type type;
    struct venu_source_location location;

    union {
        struct venu_ast_command command;

        struct venu_ast_variable_declaration
            variable_declaration;

        struct venu_ast_assignment assignment;

        struct venu_ast_expression *expression;
    } as;

    struct venu_ast_statement *next;
};

struct venu_ast_program {
    struct venu_ast_statement *first_statement;
    struct venu_ast_statement *last_statement;

    size_t statement_count;
};

#endif