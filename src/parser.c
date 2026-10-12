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

struct venu_parse_result venu_parse_command(
    struct venu_lexer *lexer,
    struct venu_command *command
)
{
    struct venu_parse_result result =
        make_result();

    if (lexer == NULL ||
        command == NULL) {
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