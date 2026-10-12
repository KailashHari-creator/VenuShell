#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "venu/arena.h"

void venu_arena_initialize(
    struct venu_arena *arena,
    void *memory,
    size_t capacity
)
{
    if (arena == NULL) {
        return;
    }

    arena->memory =
        (unsigned char *)memory;

    arena->capacity =
        memory != NULL ? capacity : 0;

    arena->used = 0;
}

void venu_arena_reset(
    struct venu_arena *arena
)
{
    if (arena == NULL) {
        return;
    }

    arena->used = 0;
}

size_t venu_arena_mark(
    const struct venu_arena *arena
)
{
    if (arena == NULL) {
        return 0;
    }

    return arena->used;
}

bool venu_arena_rewind(
    struct venu_arena *arena,
    size_t mark
)
{
    if (arena == NULL ||
        mark > arena->used) {
        return false;
    }

    arena->used = mark;

    return true;
}

void *venu_arena_allocate(
    struct venu_arena *arena,
    size_t size,
    size_t alignment
)
{
    if (arena == NULL ||
        arena->memory == NULL ||
        size == 0 ||
        arena->used > arena->capacity ||
        alignment == 0 ||
        (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    uintptr_t current_address =
        (uintptr_t)(
            arena->memory +
            arena->used
        );

    size_t misalignment =
        (size_t)(
            current_address &
            (uintptr_t)(alignment - 1)
        );

    size_t padding =
        misalignment == 0
            ? 0
            : alignment - misalignment;

    size_t remaining =
        arena->capacity - arena->used;

    if (padding > remaining) {
        return NULL;
    }

    remaining -= padding;

    if (size > remaining) {
        return NULL;
    }

    void *allocation =
        arena->memory +
        arena->used +
        padding;

    arena->used +=
        padding + size;

    memset(
        allocation,
        0,
        size
    );

    return allocation;
}