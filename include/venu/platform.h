#ifndef VENU_PLATFORM_H
#define VENU_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Every environment capable of running VENU supplies these operations.
 *
 * The language runtime does not directly use printf(), fgets(),
 * KrishnaOS syscalls, HTTP, or any particular operating-system API.
 */
struct venu_platform {
    void *context;

    int64_t (*write)(
        void *context,
        const char *data,
        size_t size
    );

    int64_t (*read_line)(
        void *context,
        char *buffer,
        size_t capacity
    );

    void (*sleep_ms)(
        void *context,
        uint64_t milliseconds
    );

    uint64_t (*process_id)(
        void *context
    );
};

/*
 * Populate a platform structure with Linux/host implementations.
 */
bool venu_host_platform_initialize(
    struct venu_platform *platform
);

#endif