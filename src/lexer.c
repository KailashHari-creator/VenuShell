#include <stdbool.h>
#include <stddef.h>

#include "venu/lexer.h"

static bool is_digit(char character)
{
    return character >= '0' &&
           character <= '9';
}

static bool is_horizontal_space(
    char character
)
{
    return character == ' '  ||
           character == '\t' ||
           character == '\r';
}

static bool is_word_terminator(
    char character
)
{
    return is_horizontal_space(character) ||
           character == '\n' ||
           character == '"' ||
           character == '\'' ||
           character == '(' ||
           character == ')' ||
           character == '{' ||
           character == '}' ||
           character == ',' ||
           character == ';' ||
           character == '$' ||
           character == '|' ||
           character == '&';
}

static bool is_operator_boundary(
    const struct venu_lexer *lexer,
    size_t position
)
{
    if (position >= lexer->length) {
        return true;
    }

    char character =
        lexer->source[position];

    return is_horizontal_space(character) ||
           character == '\n' ||
           character == '(' ||
           character == ')' ||
           character == '{' ||
           character == '}' ||
           character == ',' ||
           character == ';';
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

static void advance_lexer(
    struct venu_lexer *lexer
)
{
    if (lexer->position >= lexer->length) {
        return;
    }

    if (lexer->source[lexer->position] ==
        '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    lexer->position++;
}

static bool make_operator_token(
    struct venu_lexer *lexer,
    struct venu_token *token
)
{
    size_t start = lexer->position;
    size_t line = lexer->line;
    size_t column = lexer->column;

    char current =
        lexer->source[lexer->position];

    char next =
        lexer->position + 1 < lexer->length
            ? lexer->source[lexer->position + 1]
            : '\0';

    enum venu_token_type type;
    size_t length = 1;

    /*
     * Two-character operators.
     */
    if (current == '=' && next == '=') {
        type = VENU_TOKEN_EQUAL;
        length = 2;
    } else if (current == '!' &&
               next == '=') {
        type = VENU_TOKEN_NOT_EQUAL;
        length = 2;
    } else if (current == '<' &&
               next == '=') {
        type = VENU_TOKEN_LESS_EQUAL;
        length = 2;
    } else if (current == '>' &&
               next == '=') {
        type = VENU_TOKEN_GREATER_EQUAL;
        length = 2;
    } else if (current == '&' &&
               next == '&') {
        type = VENU_TOKEN_AND;
        length = 2;
    } else if (current == '|' &&
               next == '|') {
        type = VENU_TOKEN_OR;
        length = 2;
    } else if (current == '>' &&
               next == '>') {
        type = VENU_TOKEN_APPEND;
        length = 2;
    } else {
        /*
         * Single-character operators.
         *
         * Arithmetic, assignment and comparison operators must
         * currently be separated from words by whitespace.
         *
         * Therefore:
         *
         *     x = 10    works
         *     x=10      remains one WORD token
         */
        if ((current == '=' ||
             current == '+' ||
             current == '-' ||
             current == '*' ||
             current == '/' ||
             current == '%' ||
             current == '!' ||
             current == '<' ||
             current == '>') &&
            !is_operator_boundary(
                lexer,
                lexer->position + 1
            )) {
            return false;
        }

        switch (current) {
            case '(':
                type =
                    VENU_TOKEN_LEFT_PARENTHESIS;
                break;

            case ')':
                type =
                    VENU_TOKEN_RIGHT_PARENTHESIS;
                break;

            case '{':
                type =
                    VENU_TOKEN_LEFT_BRACE;
                break;

            case '}':
                type =
                    VENU_TOKEN_RIGHT_BRACE;
                break;

            case ',':
                type = VENU_TOKEN_COMMA;
                break;

            case ';':
                type = VENU_TOKEN_SEMICOLON;
                break;

            case '$':
                type = VENU_TOKEN_DOLLAR;
                break;

            case '+':
                type = VENU_TOKEN_PLUS;
                break;

            case '-':
                type = VENU_TOKEN_MINUS;
                break;

            case '*':
                type = VENU_TOKEN_STAR;
                break;

            case '/':
                type = VENU_TOKEN_SLASH;
                break;

            case '%':
                type = VENU_TOKEN_PERCENT;
                break;

            case '=':
                type = VENU_TOKEN_ASSIGN;
                break;

            case '!':
                type = VENU_TOKEN_NOT;
                break;

            case '<':
                type = VENU_TOKEN_LESS;
                break;

            case '>':
                type = VENU_TOKEN_GREATER;
                break;

            case '|':
                type = VENU_TOKEN_PIPE;
                break;

            case '&':
                type = VENU_TOKEN_AMPERSAND;
                break;

            default:
                return false;
        }
    }

    for (size_t index = 0;
         index < length;
         index++) {
        advance_lexer(lexer);
    }

    *token = make_token(
        lexer,
        type,
        start,
        length,
        line,
        column
    );

    return true;
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

        case VENU_TOKEN_LITERAL_STRING:
            return "LITERAL_STRING";

        case VENU_TOKEN_NUMBER:
            return "NUMBER";

        case VENU_TOKEN_NEWLINE:
            return "NEWLINE";

        case VENU_TOKEN_END:
            return "END";

        case VENU_TOKEN_LEFT_PARENTHESIS:
            return "LEFT_PARENTHESIS";

        case VENU_TOKEN_RIGHT_PARENTHESIS:
            return "RIGHT_PARENTHESIS";

        case VENU_TOKEN_LEFT_BRACE:
            return "LEFT_BRACE";

        case VENU_TOKEN_RIGHT_BRACE:
            return "RIGHT_BRACE";

        case VENU_TOKEN_COMMA:
            return "COMMA";

        case VENU_TOKEN_SEMICOLON:
            return "SEMICOLON";

        case VENU_TOKEN_DOLLAR:
            return "DOLLAR";

        case VENU_TOKEN_PLUS:
            return "PLUS";

        case VENU_TOKEN_MINUS:
            return "MINUS";

        case VENU_TOKEN_STAR:
            return "STAR";

        case VENU_TOKEN_SLASH:
            return "SLASH";

        case VENU_TOKEN_PERCENT:
            return "PERCENT";

        case VENU_TOKEN_ASSIGN:
            return "ASSIGN";

        case VENU_TOKEN_EQUAL:
            return "EQUAL";

        case VENU_TOKEN_NOT:
            return "NOT";

        case VENU_TOKEN_NOT_EQUAL:
            return "NOT_EQUAL";

        case VENU_TOKEN_LESS:
            return "LESS";

        case VENU_TOKEN_LESS_EQUAL:
            return "LESS_EQUAL";

        case VENU_TOKEN_GREATER:
            return "GREATER";

        case VENU_TOKEN_GREATER_EQUAL:
            return "GREATER_EQUAL";

        case VENU_TOKEN_PIPE:
            return "PIPE";

        case VENU_TOKEN_AND:
            return "AND";

        case VENU_TOKEN_OR:
            return "OR";

        case VENU_TOKEN_AMPERSAND:
            return "AMPERSAND";

        case VENU_TOKEN_APPEND:
            return "APPEND";

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
     * Skip whitespace and comments.
     */
    for (;;) {
        while (lexer->position < lexer->length &&
               is_horizontal_space(
                   lexer->source[lexer->position]
               )) {
            advance_lexer(lexer);
        }

        /*
         * A comment is recognised only when '#' appears where a
         * new token would begin. A '#' inside a word remains part
         * of that word.
         */
        if (lexer->position < lexer->length &&
            lexer->source[lexer->position] ==
                '#') {
            while (lexer->position < lexer->length &&
                   lexer->source[lexer->position] !=
                       '\n') {
                advance_lexer(lexer);
            }

            continue;
        }

        break;
    }

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

        advance_lexer(lexer);

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
     * Double-quoted string.
     */
    if (character == '"') {
        advance_lexer(lexer);

        size_t string_start =
            lexer->position;

        while (lexer->position < lexer->length) {
            char current =
                lexer->source[lexer->position];

            if (current == '"') {
                break;
            }

            if (current == '\n') {
                break;
            }

            /*
             * Skip an escaped character so an escaped quote does
             * not terminate the string. Escape decoding happens
             * later in the parser/evaluator.
             */
            if (current == '\\' &&
                lexer->position + 1 <
                    lexer->length &&
                lexer->source[
                    lexer->position + 1
                ] != '\n') {
                advance_lexer(lexer);
                advance_lexer(lexer);
                continue;
            }

            advance_lexer(lexer);
        }

        if (lexer->position >= lexer->length ||
            lexer->source[lexer->position] !=
                '"') {
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

        advance_lexer(lexer);

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
     * Single-quoted literal string.
     */
    if (character == '\'') {
        advance_lexer(lexer);

        size_t string_start =
            lexer->position;

        while (lexer->position < lexer->length &&
               lexer->source[lexer->position] !=
                   '\'' &&
               lexer->source[lexer->position] !=
                   '\n') {
            advance_lexer(lexer);
        }

        if (lexer->position >= lexer->length ||
            lexer->source[lexer->position] !=
                '\'') {
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

        advance_lexer(lexer);

        return make_token(
            lexer,
            VENU_TOKEN_LITERAL_STRING,
            string_start,
            string_length,
            token_line,
            token_column
        );
    }

    struct venu_token operator_token;

    if (make_operator_token(
            lexer,
            &operator_token
        )) {
        return operator_token;
    }

    /*
     * Ordinary word or integer.
     */
    size_t word_start =
        lexer->position;

    bool contains_only_digits = true;

    while (lexer->position < lexer->length &&
           !is_word_terminator(
               lexer->source[lexer->position]
           )) {
        if (!is_digit(
                lexer->source[lexer->position]
            )) {
            contains_only_digits = false;
        }

        advance_lexer(lexer);
    }

    size_t word_length =
        lexer->position - word_start;

    if (word_length == 0) {
        /*
         * Defensive fallback preventing an infinite lexer loop.
         */
        advance_lexer(lexer);

        return make_token(
            lexer,
            VENU_TOKEN_ERROR,
            word_start,
            1,
            token_line,
            token_column
        );
    }

    return make_token(
        lexer,
        contains_only_digits
            ? VENU_TOKEN_NUMBER
            : VENU_TOKEN_WORD,
        word_start,
        word_length,
        token_line,
        token_column
    );
}