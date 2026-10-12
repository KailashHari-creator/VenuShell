#ifndef VENU_RUNTIME_H
#define VENU_RUNTIME_H

#include <stdbool.h>

#include "venu/parser.h"
#include "venu/platform.h"

#define VENU_VERSION "0.1.0"

struct venu_runtime {
    struct venu_platform *platform;

    bool should_exit;
    int exit_status;
    int last_status;
};

bool venu_runtime_initialize(
    struct venu_runtime *runtime,
    struct venu_platform *platform
);

int venu_runtime_execute(
    struct venu_runtime *runtime,
    const struct venu_command *command
);

#endif