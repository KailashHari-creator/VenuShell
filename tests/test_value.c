#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "venu/platform.h"
#include "venu/value.h"

int main(void)
{
    struct venu_platform platform;

    assert(
        venu_host_platform_initialize(
            &platform
        )
    );

    char source[] = "Govinda";
    struct venu_value original;

    assert(
        venu_value_initialize_string(
            &original,
            source,
            strlen(source),
            &platform.allocator
        )
    );

    /*
     * Changing the original buffer must not affect the
     * independently owned VENU string.
     */
    source[0] = 'X';

    assert(
        strcmp(
            original.as.string.data,
            "Govinda"
        ) == 0
    );

    struct venu_value clone;

    assert(
        venu_value_clone(
            &clone,
            &original,
            &platform.allocator
        )
    );

    venu_value_destroy(
        &original,
        &platform.allocator
    );

    assert(original.type == VENU_VALUE_NULL);
    assert(clone.type == VENU_VALUE_STRING);

    /*
     * Destroying the original must not invalidate the clone.
     */
    assert(
        strcmp(
            clone.as.string.data,
            "Govinda"
        ) == 0
    );

    venu_value_destroy(
        &clone,
        &platform.allocator
    );

    assert(clone.type == VENU_VALUE_NULL);

    /*
     * Borrowed memory must not be deallocated by value_destroy().
     */
    char borrowed_data[] = "Radha";
    struct venu_value borrowed;

    assert(
        venu_value_initialize_borrowed_string(
            &borrowed,
            borrowed_data,
            strlen(borrowed_data)
        )
    );

    venu_value_destroy(
        &borrowed,
        &platform.allocator
    );

    assert(borrowed.type == VENU_VALUE_NULL);
    assert(strcmp(borrowed_data, "Radha") == 0);

    puts("Value ownership tests passed");

    return 0;
}