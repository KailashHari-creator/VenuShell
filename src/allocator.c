#include <stdbool.h>
#include <stddef.h>

#include "venu/allocator.h"

bool venu_allocator_is_valid(
    const struct venu_allocator *allocator
)
{
    return allocator != NULL &&
        allocator->allocate != NULL &&
        allocator->deallocate != NULL;
}

void *venu_allocate(
    const struct venu_allocator *allocator,
    size_t size
)
{
    if (!venu_allocator_is_valid(allocator) ||
        size == 0) {
        return NULL;
    }

    return allocator->allocate(
        allocator->context,
        size
    );
}

void venu_deallocate(
    const struct venu_allocator *allocator,
    void *memory
)
{
    if (!venu_allocator_is_valid(allocator) ||
        memory == NULL) {
        return;
    }

    allocator->deallocate(
        allocator->context,
        memory
    );
}