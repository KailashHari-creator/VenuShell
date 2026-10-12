#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "venu/lexer.h"
#include "venu/parser.h"
#include "venu/platform.h"
#include "venu/runtime.h"

#define VENU_INPUT_CAPACITY 512

static bool write_text(
    struct venu_platform *platform,
    const char *text
)
{
    if (platform == NULL ||
        platform->write == NULL ||
        text == NULL) {
        return false;
    }

    size_t length = strlen(text);

    int64_t result = platform->write(
        platform->context,
        text,
        length
    );

    return result == (int64_t)length;
}

int main(void)
{
    struct venu_platform platform;

    if (!venu_host_platform_initialize(
            &platform
        )) {
        return 1;
    }

    struct venu_runtime runtime;

    if (!venu_runtime_initialize(
            &runtime,
            &platform
        )) {
        write_text(
            &platform,
            "venu: failed to initialize runtime\n"
        );

        return 1;
    }

    write_text(
        &platform,
        "VENU " VENU_VERSION "\n"
        "Type 'help' for available commands.\n"
        "\n"
    );

    char input[VENU_INPUT_CAPACITY];

    while (!runtime.should_exit) {
        if (!write_text(
                &platform,
                "venu> "
            )) {
            return 1;
        }

        int64_t length = platform.read_line(
            platform.context,
            input,
            sizeof(input)
        );

        if (length < 0) {
            write_text(
                &platform,
                "\nvenu: input error\n"
            );

            return 1;
        }

        if (length == 0) {
            write_text(
                &platform,
                "\n"
            );

            break;
        }

        struct venu_lexer lexer;

        venu_lexer_initialize(
            &lexer,
            input,
            (size_t)length
        );

        struct venu_command command;

        struct venu_parse_result result =
            venu_parse_command(
                &lexer,
                &command
            );

        if (!result.success) {
            write_text(
                &platform,
                "venu: parse error: "
            );

            write_text(
                &platform,
                result.error
            );

            write_text(
                &platform,
                "\n"
            );

            runtime.last_status = 2;

            continue;
        }

        if (result.empty) {
            continue;
        }

        venu_runtime_execute(
            &runtime,
            &command
        );
    }

    return runtime.exit_status;
}