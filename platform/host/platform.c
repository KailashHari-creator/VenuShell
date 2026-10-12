#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "venu/platform.h"

static void *host_allocate(
    void *context,
    size_t size
)
{
    (void)context;

    return malloc(size);
}

static void host_deallocate(
    void *context,
    void *memory
)
{
    (void)context;

    free(memory);
}

static int64_t host_write(
    void *context,
    const char *data,
    size_t size
)
{
    (void)context;

    if (data == NULL && size != 0) {
        return -1;
    }

    size_t written = 0;

    while (written < size) {
        size_t result = fwrite(
            data + written,
            1,
            size - written,
            stdout
        );

        if (result == 0) {
            return -1;
        }

        written += result;
    }

    if (fflush(stdout) != 0) {
        return -1;
    }

    return (int64_t)written;
}

static int64_t host_read_line(
    void *context,
    char *buffer,
    size_t capacity
)
{
    (void)context;

    if (buffer == NULL ||
        capacity < 2 ||
        capacity > INT_MAX) {
        return -1;
    }

    if (fgets(
            buffer,
            (int)capacity,
            stdin
        ) == NULL) {
        /*
         * Zero means ordinary end-of-file.
         * Negative means an input error.
         */
        return ferror(stdin) ? -1 : 0;
    }

    return (int64_t)strlen(buffer);
}

static void host_sleep_ms(
    void *context,
    uint64_t milliseconds
)
{
    (void)context;

    struct timespec request;

    request.tv_sec =
        (time_t)(milliseconds / 1000);

    request.tv_nsec =
        (long)(
            (milliseconds % 1000) *
            1000000
        );

    while (nanosleep(
            &request,
            &request
        ) != 0) {
        if (errno != EINTR) {
            break;
        }
    }
}

static uint64_t host_process_id(
    void *context
)
{
    (void)context;

    return (uint64_t)getpid();
}

static int64_t host_execute(
    void *context,
    const char *program,
    const char *const arguments[]
)
{
    (void)context;

    if (program == NULL ||
        arguments == NULL) {
        return -1;
    }

    pid_t child = fork();

    if (child < 0) {
        return -1;
    }

    if (child == 0) {
        execvp(
            program,
            (char *const *)arguments
        );

        /*
         * execvp only returns when execution fails.
         */
        fprintf(
            stderr,
            "venu: unable to execute '%s': %s\n",
            program,
            strerror(errno)
        );

        _exit(errno == ENOENT ? 127 : 126);
    }

    int status;

    while (waitpid(
            child,
            &status,
            0
        ) < 0) {
        if (errno != EINTR) {
            return -1;
        }
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

static int64_t host_get_working_directory(
    void *context,
    char *buffer,
    size_t capacity
)
{
    (void)context;

    if (buffer == NULL ||
        capacity == 0) {
        return -1;
    }

    if (getcwd(buffer, capacity) == NULL) {
        return -1;
    }

    return (int64_t)strlen(buffer);
}

static int64_t host_change_working_directory(
    void *context,
    const char *path
)
{
    (void)context;

    if (path == NULL ||
        path[0] == '\0') {
        return -1;
    }

    return chdir(path) == 0 ? 0 : -1;
}

bool venu_host_platform_initialize(
    struct venu_platform *platform
)
{
    if (platform == NULL) {
        return false;
    }

    platform->context = NULL;
    platform->allocator.context = NULL;
    platform->allocator.allocate = host_allocate;
    platform->allocator.deallocate = host_deallocate;
    platform->write = host_write;
    platform->read_line = host_read_line;
    platform->sleep_ms = host_sleep_ms;
    platform->process_id = host_process_id;
    platform->execute = host_execute;
    platform->get_working_directory =
        host_get_working_directory;
    platform->change_working_directory =
        host_change_working_directory;

    return true;
}