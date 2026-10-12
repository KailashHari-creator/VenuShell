#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "venu/parser.h"

static struct venu_parse_result make_result(void)
{
    struct venu_parse_result result;

    result.success = false;
    result.empty = false;
    result.error[0] = '\0';

    return result;
}

static void set_error(
    struct venu_parse_result *result,
    const char *message
)
{
    if (result == NULL || message == NULL) {
        return;
    }

    size_t length = strlen(message);

    if (length >= VENU_PARSE_ERROR_CAPACITY) {
        length = VENU_PARSE_ERROR_CAPACITY - 1;
    }

    memcpy(
        result->error,
        message,
        length
    );

    result->error[length] = '\0';
}

static bool copy_token_text(
    char *destination,
    size_t capacity,
    const struct venu_token *token
)
{
    if (destination == NULL ||
        token == NULL ||
        token->start == NULL ||
        capacity == 0 ||
        token->length >= capacity) {
        return false;
    }

    memcpy(
        destination,
        token->start,
        token->length
    );

    destination[token->length] = '\0';

    return true;
}

static bool is_argument_token(
    enum venu_token_type type
)
{
    return type == VENU_TOKEN_WORD ||
           type == VENU_TOKEN_STRING ||
           type == VENU_TOKEN_LITERAL_STRING ||
           type == VENU_TOKEN_NUMBER;
}

struct venu_parse_result venu_parse_command(
    struct venu_lexer *lexer,
    struct venu_command *command
)
{
    struct venu_parse_result result =
        make_result();

    if (lexer == NULL || command == NULL) {
        set_error(
            &result,
            "invalid parser argument"
        );

        return result;
    }

    memset(
        command,
        0,
        sizeof(*command)
    );

    struct venu_token token =
        venu_lexer_next(lexer);

    /*
     * Empty input or a blank line.
     */
    if (token.type == VENU_TOKEN_END ||
        token.type == VENU_TOKEN_NEWLINE) {
        result.success = true;
        result.empty = true;

        return result;
    }

    if (token.type == VENU_TOKEN_ERROR) {
        set_error(
            &result,
            "lexer error: unterminated string"
        );

        return result;
    }

    /*
     * Commands currently begin with an ordinary word.
     */
    if (token.type != VENU_TOKEN_WORD) {
        set_error(
            &result,
            "expected command name"
        );

        return result;
    }

    if (!copy_token_text(
            command->name,
            sizeof(command->name),
            &token
        )) {
        set_error(
            &result,
            "command name is too long"
        );

        return result;
    }

    for (;;) {
        token = venu_lexer_next(lexer);

        if (token.type == VENU_TOKEN_END ||
            token.type == VENU_TOKEN_NEWLINE) {
            result.success = true;
            result.empty = false;

            return result;
        }

        if (token.type == VENU_TOKEN_ERROR) {
            set_error(
                &result,
                "lexer error: unterminated string"
            );

            return result;
        }

        if (!is_argument_token(token.type)) {
            set_error(
                &result,
                "unexpected token in argument list"
            );

            return result;
        }

        if (command->argument_count >=
            VENU_MAX_ARGUMENTS) {
            set_error(
                &result,
                "too many command arguments"
            );

            return result;
        }

        char *argument =
            command->arguments[
                command->argument_count
            ];

        if (!copy_token_text(
                argument,
                VENU_ARGUMENT_CAPACITY,
                &token
            )) {
            set_error(
                &result,
                "command argument is too long"
            );

            return result;
        }

        command->argument_count++;
    }
}
struct venu_ast_parser_state {
    struct venu_lexer *lexer;
    struct venu_arena *arena;

    struct venu_token current;

    bool failed;

    struct venu_ast_parse_result *result;
};

static bool ast_token_text_equals(
    const struct venu_token *token,
    const char *text
)
{
    if (token == NULL ||
        text == NULL) {
        return false;
    }

    size_t length = strlen(text);

    return token->length == length &&
           memcmp(
               token->start,
               text,
               length
           ) == 0;
}

static struct venu_source_location
ast_token_location(
    const struct venu_token *token
)
{
    struct venu_source_location location;

    location.line = token->line;
    location.column = token->column;

    return location;
}

static struct venu_ast_text ast_token_text(
    const struct venu_token *token
)
{
    struct venu_ast_text text;

    text.data = token->start;
    text.length = token->length;

    return text;
}

static void ast_set_error(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token,
    const char *message
)
{
    if (parser == NULL ||
        parser->failed ||
        parser->result == NULL ||
        message == NULL) {
        return;
    }

    parser->failed = true;

    parser->result->error_line =
        token != NULL ? token->line : 0;

    parser->result->error_column =
        token != NULL ? token->column : 0;

    size_t length = strlen(message);

    if (length >= VENU_PARSE_ERROR_CAPACITY) {
        length = VENU_PARSE_ERROR_CAPACITY - 1;
    }

    memcpy(
        parser->result->error,
        message,
        length
    );

    parser->result->error[length] = '\0';
}

static void ast_advance(
    struct venu_ast_parser_state *parser
)
{
    parser->current =
        venu_lexer_next(parser->lexer);
}

static bool ast_expect(
    struct venu_ast_parser_state *parser,
    enum venu_token_type type,
    const char *message
)
{
    if (parser->current.type != type) {
        ast_set_error(
            parser,
            &parser->current,
            message
        );

        return false;
    }

    ast_advance(parser);

    return true;
}

static struct venu_ast_expression *
ast_allocate_expression(
    struct venu_ast_parser_state *parser,
    enum venu_ast_expression_type type,
    struct venu_source_location location
)
{
    struct venu_ast_expression *expression =
        VENU_ARENA_NEW(
            parser->arena,
            struct venu_ast_expression
        );

    if (expression == NULL) {
        ast_set_error(
            parser,
            &parser->current,
            "AST memory exhausted"
        );

        return NULL;
    }

    expression->type = type;
    expression->location = location;

    return expression;
}

static struct venu_ast_statement *
ast_allocate_statement(
    struct venu_ast_parser_state *parser,
    enum venu_ast_statement_type type,
    struct venu_source_location location
)
{
    struct venu_ast_statement *statement =
        VENU_ARENA_NEW(
            parser->arena,
            struct venu_ast_statement
        );

    if (statement == NULL) {
        ast_set_error(
            parser,
            &parser->current,
            "AST memory exhausted"
        );

        return NULL;
    }

    statement->type = type;
    statement->location = location;

    return statement;
}

static char *ast_copy_string(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token,
    bool decode_escapes,
    size_t *output_length
)
{
    char *result = venu_arena_allocate(
        parser->arena,
        token->length + 1,
        _Alignof(char)
    );

    if (result == NULL) {
        ast_set_error(
            parser,
            token,
            "AST memory exhausted"
        );

        return NULL;
    }

    size_t source_index = 0;
    size_t destination_index = 0;

    while (source_index < token->length) {
        char character =
            token->start[source_index];

        if (decode_escapes &&
            character == '\\' &&
            source_index + 1 < token->length) {
            source_index++;

            char escaped =
                token->start[source_index];

            switch (escaped) {
                case 'n':
                    character = '\n';
                    break;

                case 't':
                    character = '\t';
                    break;

                case 'r':
                    character = '\r';
                    break;

                case '"':
                    character = '"';
                    break;

                case '\\':
                    character = '\\';
                    break;

                case '$':
                    character = '$';
                    break;

                default:
                    character = escaped;
                    break;
            }
        }

        result[destination_index++] =
            character;

        source_index++;
    }

    result[destination_index] = '\0';

    if (output_length != NULL) {
        *output_length = destination_index;
    }

    return result;
}

static struct venu_ast_expression *
ast_make_null_literal(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token
)
{
    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_LITERAL,
            ast_token_location(token)
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.literal.type =
        VENU_VALUE_NULL;

    return expression;
}

static struct venu_ast_expression *
ast_make_boolean_literal(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token,
    bool value
)
{
    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_LITERAL,
            ast_token_location(token)
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.literal.type =
        VENU_VALUE_BOOLEAN;

    expression->as.literal.as.boolean =
        value;

    return expression;
}

static struct venu_ast_expression *
ast_make_integer_literal(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token
)
{
    uint64_t value = 0;

    for (size_t index = 0;
         index < token->length;
         index++) {
        uint64_t digit =
            (uint64_t)(
                token->start[index] - '0'
            );

        if (value >
            ((uint64_t)INT64_MAX - digit) / 10) {
            ast_set_error(
                parser,
                token,
                "integer literal is too large"
            );

            return NULL;
        }

        value = value * 10 + digit;
    }

    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_LITERAL,
            ast_token_location(token)
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.literal.type =
        VENU_VALUE_INTEGER;

    expression->as.literal.as.integer =
        (int64_t)value;

    return expression;
}

static struct venu_ast_expression *
ast_make_string_literal(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token,
    bool decode_escapes
)
{
    size_t length;

    char *data = ast_copy_string(
        parser,
        token,
        decode_escapes,
        &length
    );

    if (data == NULL) {
        return NULL;
    }

    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_LITERAL,
            ast_token_location(token)
        );

    if (expression == NULL) {
        return NULL;
    }

    if (!venu_value_initialize_borrowed_string(
            &expression->as.literal,
            data,
            length
        )) {
        ast_set_error(
            parser,
            token,
            "failed to create string literal"
        );

        return NULL;
    }

    return expression;
}

static struct venu_ast_expression *
ast_make_variable_expression(
    struct venu_ast_parser_state *parser,
    const struct venu_token *token
)
{
    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_VARIABLE,
            ast_token_location(token)
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.variable =
        ast_token_text(token);

    return expression;
}

static struct venu_ast_expression *
ast_make_binary_expression(
    struct venu_ast_parser_state *parser,
    enum venu_ast_binary_operator operator,
    struct venu_source_location location,
    struct venu_ast_expression *left,
    struct venu_ast_expression *right
)
{
    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_BINARY,
            location
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.binary.operator =
        operator;

    expression->as.binary.left = left;
    expression->as.binary.right = right;

    return expression;
}

static struct venu_ast_expression *
ast_parse_expression(
    struct venu_ast_parser_state *parser
);

static struct venu_ast_expression *
ast_parse_primary(
    struct venu_ast_parser_state *parser
)
{
    struct venu_token token =
        parser->current;

    if (token.type == VENU_TOKEN_ERROR) {
        ast_set_error(
            parser,
            &token,
            "lexer error: unterminated string"
        );

        return NULL;
    }

    if (token.type == VENU_TOKEN_NUMBER) {
        ast_advance(parser);

        return ast_make_integer_literal(
            parser,
            &token
        );
    }

    if (token.type == VENU_TOKEN_STRING) {
        ast_advance(parser);

        return ast_make_string_literal(
            parser,
            &token,
            true
        );
    }

    if (token.type ==
        VENU_TOKEN_LITERAL_STRING) {
        ast_advance(parser);

        return ast_make_string_literal(
            parser,
            &token,
            false
        );
    }

    if (token.type == VENU_TOKEN_DOLLAR) {
        ast_advance(parser);

        if (parser->current.type !=
            VENU_TOKEN_WORD) {
            ast_set_error(
                parser,
                &parser->current,
                "expected variable name after '$'"
            );

            return NULL;
        }

        token = parser->current;

        ast_advance(parser);

        return ast_make_variable_expression(
            parser,
            &token
        );
    }

    if (token.type == VENU_TOKEN_WORD) {
        ast_advance(parser);

        if (ast_token_text_equals(
                &token,
                "true"
            )) {
            return ast_make_boolean_literal(
                parser,
                &token,
                true
            );
        }

        if (ast_token_text_equals(
                &token,
                "false"
            )) {
            return ast_make_boolean_literal(
                parser,
                &token,
                false
            );
        }

        if (ast_token_text_equals(
                &token,
                "null"
            )) {
            return ast_make_null_literal(
                parser,
                &token
            );
        }

        return ast_make_variable_expression(
            parser,
            &token
        );
    }

    if (token.type ==
        VENU_TOKEN_LEFT_PARENTHESIS) {
        ast_advance(parser);

        struct venu_ast_expression *expression =
            ast_parse_expression(parser);

        if (expression == NULL) {
            return NULL;
        }

        if (!ast_expect(
                parser,
                VENU_TOKEN_RIGHT_PARENTHESIS,
                "expected ')' after expression"
            )) {
            return NULL;
        }

        return expression;
    }

    ast_set_error(
        parser,
        &token,
        "expected expression"
    );

    return NULL;
}

static struct venu_ast_expression *
ast_parse_unary(
    struct venu_ast_parser_state *parser
)
{
    struct venu_token operator_token =
        parser->current;

    bool is_not =
        operator_token.type == VENU_TOKEN_NOT ||
        (
            operator_token.type ==
                VENU_TOKEN_WORD &&
            ast_token_text_equals(
                &operator_token,
                "not"
            )
        );

    bool is_negative =
        operator_token.type ==
            VENU_TOKEN_MINUS;

    if (!is_not && !is_negative) {
        return ast_parse_primary(parser);
    }

    ast_advance(parser);

    struct venu_ast_expression *operand =
        ast_parse_unary(parser);

    if (operand == NULL) {
        return NULL;
    }

    struct venu_ast_expression *expression =
        ast_allocate_expression(
            parser,
            VENU_AST_EXPRESSION_UNARY,
            ast_token_location(
                &operator_token
            )
        );

    if (expression == NULL) {
        return NULL;
    }

    expression->as.unary.operator =
        is_not
            ? VENU_AST_UNARY_NOT
            : VENU_AST_UNARY_NEGATE;

    expression->as.unary.operand =
        operand;

    return expression;
}

static struct venu_ast_expression *
ast_parse_factor(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_unary(parser);

    if (left == NULL) {
        return NULL;
    }

    for (;;) {
        enum venu_ast_binary_operator operator;

        if (parser->current.type ==
            VENU_TOKEN_STAR) {
            operator =
                VENU_AST_BINARY_MULTIPLY;
        } else if (parser->current.type ==
                   VENU_TOKEN_SLASH) {
            operator =
                VENU_AST_BINARY_DIVIDE;
        } else if (parser->current.type ==
                   VENU_TOKEN_PERCENT) {
            operator =
                VENU_AST_BINARY_REMAINDER;
        } else {
            break;
        }

        struct venu_token operator_token =
            parser->current;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_unary(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            operator,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static struct venu_ast_expression *
ast_parse_term(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_factor(parser);

    if (left == NULL) {
        return NULL;
    }

    while (parser->current.type ==
               VENU_TOKEN_PLUS ||
           parser->current.type ==
               VENU_TOKEN_MINUS) {
        struct venu_token operator_token =
            parser->current;

        enum venu_ast_binary_operator operator =
            operator_token.type ==
                VENU_TOKEN_PLUS
                ? VENU_AST_BINARY_ADD
                : VENU_AST_BINARY_SUBTRACT;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_factor(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            operator,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static struct venu_ast_expression *
ast_parse_comparison(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_term(parser);

    if (left == NULL) {
        return NULL;
    }

    for (;;) {
        enum venu_ast_binary_operator operator;

        switch (parser->current.type) {
            case VENU_TOKEN_LESS:
                operator =
                    VENU_AST_BINARY_LESS;
                break;

            case VENU_TOKEN_LESS_EQUAL:
                operator =
                    VENU_AST_BINARY_LESS_EQUAL;
                break;

            case VENU_TOKEN_GREATER:
                operator =
                    VENU_AST_BINARY_GREATER;
                break;

            case VENU_TOKEN_GREATER_EQUAL:
                operator =
                    VENU_AST_BINARY_GREATER_EQUAL;
                break;

            default:
                return left;
        }

        struct venu_token operator_token =
            parser->current;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_term(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            operator,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }
}

static struct venu_ast_expression *
ast_parse_equality(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_comparison(parser);

    if (left == NULL) {
        return NULL;
    }

    while (parser->current.type ==
               VENU_TOKEN_EQUAL ||
           parser->current.type ==
               VENU_TOKEN_NOT_EQUAL) {
        struct venu_token operator_token =
            parser->current;

        enum venu_ast_binary_operator operator =
            operator_token.type ==
                VENU_TOKEN_EQUAL
                ? VENU_AST_BINARY_EQUAL
                : VENU_AST_BINARY_NOT_EQUAL;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_comparison(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            operator,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static struct venu_ast_expression *
ast_parse_and(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_equality(parser);

    if (left == NULL) {
        return NULL;
    }

    while (parser->current.type ==
               VENU_TOKEN_WORD &&
           ast_token_text_equals(
               &parser->current,
               "and"
           )) {
        struct venu_token operator_token =
            parser->current;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_equality(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            VENU_AST_BINARY_AND,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static struct venu_ast_expression *
ast_parse_or(
    struct venu_ast_parser_state *parser
)
{
    struct venu_ast_expression *left =
        ast_parse_and(parser);

    if (left == NULL) {
        return NULL;
    }

    while (parser->current.type ==
               VENU_TOKEN_WORD &&
           ast_token_text_equals(
               &parser->current,
               "or"
           )) {
        struct venu_token operator_token =
            parser->current;

        ast_advance(parser);

        struct venu_ast_expression *right =
            ast_parse_and(parser);

        if (right == NULL) {
            return NULL;
        }

        left = ast_make_binary_expression(
            parser,
            VENU_AST_BINARY_OR,
            ast_token_location(
                &operator_token
            ),
            left,
            right
        );

        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static struct venu_ast_expression *
ast_parse_expression(
    struct venu_ast_parser_state *parser
)
{
    return ast_parse_or(parser);
}

static struct venu_ast_expression *
ast_parse_command_argument(
    struct venu_ast_parser_state *parser
)
{
    struct venu_token token =
        parser->current;

    if (token.type == VENU_TOKEN_DOLLAR) {
        ast_advance(parser);

        if (parser->current.type !=
            VENU_TOKEN_WORD) {
            ast_set_error(
                parser,
                &parser->current,
                "expected variable name after '$'"
            );

            return NULL;
        }

        token = parser->current;

        ast_advance(parser);

        return ast_make_variable_expression(
            parser,
            &token
        );
    }

    if (token.type == VENU_TOKEN_NUMBER) {
        ast_advance(parser);

        return ast_make_integer_literal(
            parser,
            &token
        );
    }

    if (token.type == VENU_TOKEN_STRING) {
        ast_advance(parser);

        return ast_make_string_literal(
            parser,
            &token,
            true
        );
    }

    if (token.type ==
        VENU_TOKEN_LITERAL_STRING) {
        ast_advance(parser);

        return ast_make_string_literal(
            parser,
            &token,
            false
        );
    }

    if (token.type == VENU_TOKEN_WORD ||
        token.type == VENU_TOKEN_SLASH) {
        ast_advance(parser);

        return ast_make_string_literal(
            parser,
            &token,
            false
        );
    }

    ast_set_error(
        parser,
        &token,
        "unexpected token in command arguments"
    );

    return NULL;
}

static struct venu_ast_statement *
ast_parse_command_statement(
    struct venu_ast_parser_state *parser,
    const struct venu_token *name_token
)
{
    struct venu_ast_statement *statement =
        ast_allocate_statement(
            parser,
            VENU_AST_STATEMENT_COMMAND,
            ast_token_location(name_token)
        );

    if (statement == NULL) {
        return NULL;
    }

    statement->as.command.name =
        ast_token_text(name_token);

    while (parser->current.type !=
               VENU_TOKEN_NEWLINE &&
           parser->current.type !=
               VENU_TOKEN_SEMICOLON &&
           parser->current.type !=
               VENU_TOKEN_END) {
        struct venu_ast_expression *expression =
            ast_parse_command_argument(parser);

        if (expression == NULL) {
            return NULL;
        }

        struct venu_ast_argument *argument =
            VENU_ARENA_NEW(
                parser->arena,
                struct venu_ast_argument
            );

        if (argument == NULL) {
            ast_set_error(
                parser,
                &parser->current,
                "AST memory exhausted"
            );

            return NULL;
        }

        argument->expression = expression;

        if (statement->as.command.first_argument ==
            NULL) {
            statement->as.command.first_argument =
                argument;
        } else {
            statement->as.command.last_argument->
                next = argument;
        }

        statement->as.command.last_argument =
            argument;

        statement->as.command.argument_count++;
    }

    return statement;
}

static struct venu_ast_statement *
ast_parse_variable_declaration(
    struct venu_ast_parser_state *parser,
    const struct venu_token *let_token
)
{
    if (parser->current.type !=
        VENU_TOKEN_WORD) {
        ast_set_error(
            parser,
            &parser->current,
            "expected variable name after 'let'"
        );

        return NULL;
    }

    struct venu_token name_token =
        parser->current;

    ast_advance(parser);

    if (!ast_expect(
            parser,
            VENU_TOKEN_ASSIGN,
            "expected '=' after variable name"
        )) {
        return NULL;
    }

    struct venu_ast_expression *initializer =
        ast_parse_expression(parser);

    if (initializer == NULL) {
        return NULL;
    }

    struct venu_ast_statement *statement =
        ast_allocate_statement(
            parser,
            VENU_AST_STATEMENT_VARIABLE_DECLARATION,
            ast_token_location(let_token)
        );

    if (statement == NULL) {
        return NULL;
    }

    statement->as.variable_declaration.name =
        ast_token_text(&name_token);

    statement->as.variable_declaration.initializer =
        initializer;

    return statement;
}

static struct venu_ast_statement *
ast_parse_assignment(
    struct venu_ast_parser_state *parser,
    const struct venu_token *name_token
)
{
    ast_advance(parser);

    struct venu_ast_expression *value =
        ast_parse_expression(parser);

    if (value == NULL) {
        return NULL;
    }

    struct venu_ast_statement *statement =
        ast_allocate_statement(
            parser,
            VENU_AST_STATEMENT_ASSIGNMENT,
            ast_token_location(name_token)
        );

    if (statement == NULL) {
        return NULL;
    }

    statement->as.assignment.name =
        ast_token_text(name_token);

    statement->as.assignment.value =
        value;

    return statement;
}

static struct venu_ast_statement *
ast_parse_statement(
    struct venu_ast_parser_state *parser
)
{
    if (parser->current.type ==
        VENU_TOKEN_ERROR) {
        ast_set_error(
            parser,
            &parser->current,
            "lexer error"
        );

        return NULL;
    }

    if (parser->current.type !=
        VENU_TOKEN_WORD) {
        ast_set_error(
            parser,
            &parser->current,
            "expected statement"
        );

        return NULL;
    }

    struct venu_token first_token =
        parser->current;

    ast_advance(parser);

    if (ast_token_text_equals(
            &first_token,
            "let"
        )) {
        return ast_parse_variable_declaration(
            parser,
            &first_token
        );
    }

    if (parser->current.type ==
        VENU_TOKEN_ASSIGN) {
        return ast_parse_assignment(
            parser,
            &first_token
        );
    }

    return ast_parse_command_statement(
        parser,
        &first_token
    );
}

struct venu_ast_parse_result venu_parse_program(
    struct venu_lexer *lexer,
    struct venu_arena *arena
)
{
    struct venu_ast_parse_result result;

    memset(
        &result,
        0,
        sizeof(result)
    );

    if (lexer == NULL ||
        arena == NULL) {
        memcpy(
            result.error,
            "invalid AST parser argument",
            sizeof(
                "invalid AST parser argument"
            )
        );

        return result;
    }

    size_t arena_mark =
        venu_arena_mark(arena);

    struct venu_ast_program *program =
        VENU_ARENA_NEW(
            arena,
            struct venu_ast_program
        );

    if (program == NULL) {
        memcpy(
            result.error,
            "AST memory exhausted",
            sizeof("AST memory exhausted")
        );

        return result;
    }

    struct venu_ast_parser_state parser;

    parser.lexer = lexer;
    parser.arena = arena;
    parser.failed = false;
    parser.result = &result;

    ast_advance(&parser);

    while (parser.current.type !=
           VENU_TOKEN_END) {
        while (parser.current.type ==
                   VENU_TOKEN_NEWLINE ||
               parser.current.type ==
                   VENU_TOKEN_SEMICOLON) {
            ast_advance(&parser);
        }

        if (parser.current.type ==
            VENU_TOKEN_END) {
            break;
        }

        struct venu_ast_statement *statement =
            ast_parse_statement(&parser);

        if (statement == NULL ||
            parser.failed) {
            venu_arena_rewind(
                arena,
                arena_mark
            );

            result.program = NULL;
            result.success = false;

            return result;
        }

        if (program->first_statement == NULL) {
            program->first_statement =
                statement;
        } else {
            program->last_statement->next =
                statement;
        }

        program->last_statement = statement;
        program->statement_count++;

        if (parser.current.type !=
                VENU_TOKEN_NEWLINE &&
            parser.current.type !=
                VENU_TOKEN_SEMICOLON &&
            parser.current.type !=
                VENU_TOKEN_END) {
            ast_set_error(
                &parser,
                &parser.current,
                "expected newline or ';' after statement"
            );

            venu_arena_rewind(
                arena,
                arena_mark
            );

            result.program = NULL;
            result.success = false;

            return result;
        }
    }

    result.success = true;
    result.program = program;

    return result;
}