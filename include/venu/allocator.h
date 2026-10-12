#ifndef VENU_ALLOCATOR_H
#define VENU_ALLOCATOR_H

#include <stdbool.h>
#include <stddef.h>

struct venu_allocator {
    void *context;

    void *(*allocate)(
        void *context,
        size_t size
    );

    void (*deallocate)(
        void *context,
        void *memory
    );
};

bool venu_allocator_is_valid(
    const struct venu_allocator *allocator
);

void *venu_allocate(
    const struct venu_allocator *allocator,
    size_t size
);

void venu_deallocate(
    const struct venu_allocator *allocator,
    void *memory
);

#endif