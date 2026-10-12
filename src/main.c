#include <stdio.h>
#include <string.h>

#include "venu/parser.h"

int main(void)
{
    char input[512];

    while (fgets(
            input,
            sizeof(input),
            stdin
        ) != NULL) {
        struct venu_lexer lexer;
        struct venu_command command;

        venu_lexer_initialize(
            &lexer,
            input,
            strlen(input)
        );

        struct venu_parse_result result =
            venu_parse_command(
                &lexer,
                &command
            );

        if (!result.success) {
            printf(
                "Parse error: %s\n",
                result.error
            );

            continue;
        }

        if (result.empty) {
            printf("Empty command\n");
            continue;
        }

        printf(
            "Command: %s\n",
            command.name
        );

        printf(
            "Arguments: %zu\n",
            command.argument_count
        );

        for (size_t index = 0;
             index < command.argument_count;
             index++) {
            printf(
                "  [%zu]: %s\n",
                index,
                command.arguments[index]
            );
        }
    }

    return 0;
}