#include <stdbool.h>
#include <stddef.h>

#include "venu/lexer.h"

static bool is_digit(char character)
{
    return character >= '0' &&
           character <= '9';
}

static bool is_horizontal_space(char character)
{
    return character == ' '  ||
           character == '\t' ||
           character == '\r';
}

static bool is_token_terminator(char character)
{
    return is_horizontal_space(character) ||
           character == '\n' ||
           character == '"';
}

static struct venu_token make_token(
    const struct venu_lexer *lexer,
    enum venu_token_type type,
    size_t start,
    size_t length,
    size_t line,
    size_t column
)
{
    struct venu_token token;

    token.type = type;
    token.start = lexer->source + start;
    token.length = length;
    token.line = line;
    token.column = column;

    return token;
}

const char *venu_token_type_name(
    enum venu_token_type type
)
{
    switch (type) {
        case VENU_TOKEN_WORD:
            return "WORD";

        case VENU_TOKEN_STRING:
            return "STRING";

        case VENU_TOKEN_NUMBER:
            return "NUMBER";

        case VENU_TOKEN_NEWLINE:
            return "NEWLINE";

        case VENU_TOKEN_END:
            return "END";

        case VENU_TOKEN_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}

void venu_lexer_initialize(
    struct venu_lexer *lexer,
    const char *source,
    size_t length
)
{
    if (lexer == NULL) {
        return;
    }

    if (source == NULL) {
        source = "";
        length = 0;
    }

    lexer->source = source;
    lexer->length = length;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

struct venu_token venu_lexer_next(
    struct venu_lexer *lexer
)
{
    /*
     * Skip leading horizontal whitespace.
     *
     * Do not skip '\n' because NEWLINE is a meaningful token.
     */
    while (lexer->position < lexer->length &&
           is_horizontal_space(
               lexer->source[lexer->position]
           )) {
        lexer->position++;
        lexer->column++;
    }

    /*
     * End of input.
     */
    if (lexer->position >= lexer->length) {
        return make_token(
            lexer,
            VENU_TOKEN_END,
            lexer->position,
            0,
            lexer->line,
            lexer->column
        );
    }

    size_t token_line = lexer->line;
    size_t token_column = lexer->column;

    char character =
        lexer->source[lexer->position];

    /*
     * Newline.
     */
    if (character == '\n') {
        size_t start = lexer->position;

        lexer->position++;
        lexer->line++;
        lexer->column = 1;

        return make_token(
            lexer,
            VENU_TOKEN_NEWLINE,
            start,
            1,
            token_line,
            token_column
        );
    }

    /*
     * Quoted string.
     */
    if (character == '"') {
        /*
         * Consume the opening quote.
         */
        lexer->position++;
        lexer->column++;

        size_t string_start =
            lexer->position;

        while (lexer->position < lexer->length &&
               lexer->source[lexer->position] != '"' &&
               lexer->source[lexer->position] != '\n') {
            lexer->position++;
            lexer->column++;
        }

        /*
         * We reached the end or a newline without finding
         * the closing quote.
         */
        if (lexer->position >= lexer->length ||
            lexer->source[lexer->position] != '"') {
            return make_token(
                lexer,
                VENU_TOKEN_ERROR,
                string_start,
                lexer->position - string_start,
                token_line,
                token_column
            );
        }

        size_t string_length =
            lexer->position - string_start;

        /*
         * Consume the closing quote.
         */
        lexer->position++;
        lexer->column++;

        return make_token(
            lexer,
            VENU_TOKEN_STRING,
            string_start,
            string_length,
            token_line,
            token_column
        );
    }

    /*
     * Ordinary word or number.
     */
    size_t word_start =
        lexer->position;

    bool contains_only_digits = true;

    while (lexer->position < lexer->length &&
           !is_token_terminator(
               lexer->source[lexer->position]
           )) {
        if (!is_digit(
                lexer->source[lexer->position]
            )) {
            contains_only_digits = false;
        }

        lexer->position++;
        lexer->column++;
    }

    size_t word_length =
        lexer->position - word_start;

    enum venu_token_type type =
        contains_only_digits
            ? VENU_TOKEN_NUMBER
            : VENU_TOKEN_WORD;

    return make_token(
        lexer,
        type,
        word_start,
        word_length,
        token_line,
        token_column
    );
}