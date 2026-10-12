#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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