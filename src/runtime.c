#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "venu/runtime.h"

#define VENU_PATH_CAPACITY 4096

typedef int (*venu_builtin_handler)(
    struct venu_runtime *runtime,
    const struct venu_command *command
);

struct venu_builtin {
    const char *name;
    venu_builtin_handler handler;
};

static int runtime_write_data(
    struct venu_runtime *runtime,
    const char *data,
    size_t size
)
{
    if (runtime == NULL ||
        runtime->platform == NULL ||
        runtime->platform->write == NULL ||
        (data == NULL && size != 0)) {
        return 1;
    }

    int64_t result = runtime->platform->write(
        runtime->platform->context,
        data,
        size
    );

    return result == (int64_t)size ? 0 : 1;
}

static int runtime_write_text(
    struct venu_runtime *runtime,
    const char *text
)
{
    if (text == NULL) {
        return 1;
    }

    return runtime_write_data(
        runtime,
        text,
        strlen(text)
    );
}

static int runtime_write_unsigned(
    struct venu_runtime *runtime,
    uint64_t value
)
{
    char buffer[21];
    size_t position = sizeof(buffer);

    if (value == 0) {
        return runtime_write_text(
            runtime,
            "0"
        );
    }

    while (value != 0) {
        uint64_t digit = value % 10;

        position--;

        buffer[position] =
            (char)('0' + digit);

        value /= 10;
    }

    return runtime_write_data(
        runtime,
        buffer + position,
        sizeof(buffer) - position
    );
}

static bool parse_unsigned(
    const char *text,
    uint64_t *value
)
{
    if (text == NULL ||
        value == NULL ||
        text[0] == '\0') {
        return false;
    }

    uint64_t result = 0;

    for (size_t index = 0;
         text[index] != '\0';
         index++) {
        char character = text[index];

        if (character < '0' ||
            character > '9') {
            return false;
        }

        uint64_t digit =
            (uint64_t)(character - '0');

        if (result >
            (UINT64_MAX - digit) / 10) {
            return false;
        }

        result = result * 10 + digit;
    }

    *value = result;

    return true;
}

static int builtin_help(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 0) {
        runtime_write_text(
            runtime,
            "usage: help\n"
        );

        return 2;
    }

    static const char help_text[] =
        "VENU - Versatile Execution and Navigation Utility\n"
        "\n"
        "Available commands:\n"
        "  help                 Show this help\n"
        "  echo [arguments...]  Print arguments\n"
        "  version              Show VENU version\n"
        "  pid                  Show shell process ID\n"
        "  pwd                  Show working directory\n"
        "  cd <directory>       Change working directory\n"
        "  status               Show previous command status\n"
        "  sleep <milliseconds> Pause execution\n"
        "  exit [status]        Exit VENU\n";

    return runtime_write_data(
        runtime,
        help_text,
        sizeof(help_text) - 1
    );
}

static int builtin_echo(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    for (size_t index = 0;
         index < command->argument_count;
         index++) {
        if (index != 0 &&
            runtime_write_text(runtime, " ") != 0) {
            return 1;
        }

        if (runtime_write_text(
                runtime,
                command->arguments[index]
            ) != 0) {
            return 1;
        }
    }

    return runtime_write_text(
        runtime,
        "\n"
    );
}

static int builtin_version(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 0) {
        runtime_write_text(
            runtime,
            "usage: version\n"
        );

        return 2;
    }

    return runtime_write_text(
        runtime,
        "VENU " VENU_VERSION "\n"
    );
}

static int builtin_pid(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 0) {
        runtime_write_text(
            runtime,
            "usage: pid\n"
        );

        return 2;
    }

    if (runtime->platform->process_id == NULL) {
        runtime_write_text(
            runtime,
            "venu: process ID is unavailable\n"
        );

        return 1;
    }

    uint64_t process_id =
        runtime->platform->process_id(
            runtime->platform->context
        );

    if (runtime_write_unsigned(
            runtime,
            process_id
        ) != 0) {
        return 1;
    }

    return runtime_write_text(
        runtime,
        "\n"
    );
}

static int builtin_sleep(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 1) {
        runtime_write_text(
            runtime,
            "usage: sleep <milliseconds>\n"
        );

        return 2;
    }

    uint64_t milliseconds;

    if (!parse_unsigned(
            command->arguments[0],
            &milliseconds
        )) {
        runtime_write_text(
            runtime,
            "venu: sleep: invalid duration\n"
        );

        return 2;
    }

    if (runtime->platform->sleep_ms == NULL) {
        runtime_write_text(
            runtime,
            "venu: sleep is unavailable\n"
        );

        return 1;
    }

    runtime->platform->sleep_ms(
        runtime->platform->context,
        milliseconds
    );

    return 0;
}

static int builtin_pwd(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 0) {
        runtime_write_text(
            runtime,
            "usage: pwd\n"
        );

        return 2;
    }

    if (runtime->platform->
            get_working_directory == NULL) {
        runtime_write_text(
            runtime,
            "venu: working directory is unavailable\n"
        );

        return 1;
    }

    char path[VENU_PATH_CAPACITY];

    int64_t length =
        runtime->platform->
            get_working_directory(
                runtime->platform->context,
                path,
                sizeof(path)
            );

    if (length < 0) {
        runtime_write_text(
            runtime,
            "venu: failed to read working directory\n"
        );

        return 1;
    }

    if (runtime_write_data(
            runtime,
            path,
            (size_t)length
        ) != 0) {
        return 1;
    }

    return runtime_write_text(
        runtime,
        "\n"
    );
}

static int builtin_cd(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 1) {
        runtime_write_text(
            runtime,
            "usage: cd <directory>\n"
        );

        return 2;
    }

    if (runtime->platform->
            change_working_directory == NULL) {
        runtime_write_text(
            runtime,
            "venu: changing directory is unavailable\n"
        );

        return 1;
    }

    int64_t result =
        runtime->platform->
            change_working_directory(
                runtime->platform->context,
                command->arguments[0]
            );

    if (result < 0) {
        runtime_write_text(
            runtime,
            "venu: cd: unable to enter '"
        );

        runtime_write_text(
            runtime,
            command->arguments[0]
        );

        runtime_write_text(
            runtime,
            "'\n"
        );

        return 1;
    }

    return 0;
}

static int builtin_status(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count != 0) {
        runtime_write_text(
            runtime,
            "usage: status\n"
        );

        return 2;
    }

    int previous_status =
        runtime->last_status;

    if (runtime_write_unsigned(
            runtime,
            (uint64_t)previous_status
        ) != 0) {
        return 1;
    }

    return runtime_write_text(
        runtime,
        "\n"
    );
}

static int builtin_exit(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (command->argument_count > 1) {
        runtime_write_text(
            runtime,
            "usage: exit [status]\n"
        );

        return 2;
    }

    int status = runtime->last_status;

    if (command->argument_count == 1) {
        uint64_t parsed_status;

        if (!parse_unsigned(
                command->arguments[0],
                &parsed_status
            ) ||
            parsed_status > 255) {
            runtime_write_text(
                runtime,
                "venu: exit: status must be between 0 and 255\n"
            );

            return 2;
        }

        status = (int)parsed_status;
    }

    runtime->should_exit = true;
    runtime->exit_status = status;

    return status;
}

static const struct venu_builtin builtins[] = {
    { "help",    builtin_help },
    { "echo",    builtin_echo },
    { "version", builtin_version },
    { "pid",     builtin_pid },
    { "pwd",     builtin_pwd },
    { "cd",      builtin_cd },
    { "status",  builtin_status },
    { "sleep",   builtin_sleep },
    { "exit",    builtin_exit }
};

bool venu_runtime_initialize(
    struct venu_runtime *runtime,
    struct venu_platform *platform
)
{
    if (runtime == NULL ||
        platform == NULL ||
        platform->write == NULL ||
        platform->read_line == NULL) {
        return false;
    }

    runtime->platform = platform;
    runtime->should_exit = false;
    runtime->exit_status = 0;
    runtime->last_status = 0;

    return true;
}

int venu_runtime_execute(
    struct venu_runtime *runtime,
    const struct venu_command *command
)
{
    if (runtime == NULL ||
        command == NULL ||
        runtime->platform == NULL) {
        return 1;
    }

    size_t builtin_count =
        sizeof(builtins) /
        sizeof(builtins[0]);

    for (size_t index = 0;
         index < builtin_count;
         index++) {
        if (strcmp(
                command->name,
                builtins[index].name
            ) == 0) {
            int status =
                builtins[index].handler(
                    runtime,
                    command
                );

            runtime->last_status = status;

            return status;
        }
    }

    if (runtime->platform->execute == NULL) {
        runtime_write_text(
            runtime,
            "venu: external command execution is unavailable\n"
        );

        runtime->last_status = 126;

        return 126;
    }

    const char *arguments[
        VENU_MAX_ARGUMENTS + 2
    ];

    arguments[0] = command->name;

    for (size_t index = 0;
        index < command->argument_count;
        index++) {
        arguments[index + 1] =
            command->arguments[index];
    }

    arguments[
        command->argument_count + 1
    ] = NULL;

    int64_t execution_status =
        runtime->platform->execute(
            runtime->platform->context,
            command->name,
            arguments
        );

    if (execution_status < 0) {
        runtime_write_text(
            runtime,
            "venu: failed to start external command\n"
        );

        runtime->last_status = 126;

        return 126;
    }

    runtime->last_status =
        (int)execution_status;

    return runtime->last_status;
}