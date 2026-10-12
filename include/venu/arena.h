#ifndef VENU_ARENA_H
#define VENU_ARENA_H

#include <stdbool.h>
#include <stddef.h>

struct venu_arena {
    unsigned char *memory;
    size_t capacity;
    size_t used;
};

void venu_arena_initialize(
    struct venu_arena *arena,
    void *memory,
    size_t capacity
);

void venu_arena_reset(
    struct venu_arena *arena
);

size_t venu_arena_mark(
    const struct venu_arena *arena
);

bool venu_arena_rewind(
    struct venu_arena *arena,
    size_t mark
);

void *venu_arena_allocate(
    struct venu_arena *arena,
    size_t size,
    size_t alignment
);

#define VENU_ARENA_NEW(arena, type) \
    ((type *)venu_arena_allocate(    \
        (arena),                     \
        sizeof(type),                \
        _Alignof(type)               \
    ))

#endif