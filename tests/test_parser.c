#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "venu/arena.h"
#include "venu/ast.h"
#include "venu/lexer.h"
#include "venu/parser.h"

static bool text_equals(
    struct venu_ast_text text,
    const char *expected
)
{
    size_t length = strlen(expected);

    return text.length == length &&
           memcmp(
               text.data,
               expected,
               length
           ) == 0;
}

int main(void)
{
    static const char source[] =
        "let count = 6 + 10\n"
        "name = \"Govinda\"\n"
        "echo $name\n";

    unsigned char memory[16384];

    struct venu_arena arena;

    venu_arena_initialize(
        &arena,
        memory,
        sizeof(memory)
    );

    struct venu_lexer lexer;

    venu_lexer_initialize(
        &lexer,
        source,
        sizeof(source) - 1
    );

    struct venu_ast_parse_result result =
        venu_parse_program(
            &lexer,
            &arena
        );

    if (!result.success) {
        fprintf(
            stderr,
            "Parse error at %zu:%zu: %s\n",
            result.error_line,
            result.error_column,
            result.error
        );

        return 1;
    }

    assert(result.program != NULL);
    assert(result.program->statement_count == 3);

    struct venu_ast_statement *statement =
        result.program->first_statement;

    assert(
        statement->type ==
        VENU_AST_STATEMENT_VARIABLE_DECLARATION
    );

    assert(
        text_equals(
            statement->
                as.variable_declaration.name,
            "count"
        )
    );

    assert(
        statement->
            as.variable_declaration.initializer->
            type ==
        VENU_AST_EXPRESSION_BINARY
    );

    statement = statement->next;

    assert(
        statement->type ==
        VENU_AST_STATEMENT_ASSIGNMENT
    );

    assert(
        text_equals(
            statement->as.assignment.name,
            "name"
        )
    );

    statement = statement->next;

    assert(
        statement->type ==
        VENU_AST_STATEMENT_COMMAND
    );

    assert(
        text_equals(
            statement->as.command.name,
            "echo"
        )
    );

    assert(
        statement->as.command.argument_count ==
        1
    );

    assert(
        statement->as.command.first_argument->
            expression->type ==
        VENU_AST_EXPRESSION_VARIABLE
    );

    puts("AST parser tests passed");

    return 0;
}