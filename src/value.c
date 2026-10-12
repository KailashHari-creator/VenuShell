#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "venu/value.h"

void venu_value_initialize_null(
    struct venu_value *value
)
{
    if (value == NULL) {
        return;
    }

    value->type = VENU_VALUE_NULL;
    value->as.integer = 0;
}

void venu_value_initialize_boolean(
    struct venu_value *value,
    bool boolean
)
{
    if (value == NULL) {
        return;
    }

    value->type = VENU_VALUE_BOOLEAN;
    value->as.boolean = boolean;
}

void venu_value_initialize_integer(
    struct venu_value *value,
    int64_t integer
)
{
    if (value == NULL) {
        return;
    }

    value->type = VENU_VALUE_INTEGER;
    value->as.integer = integer;
}

void venu_value_initialize_float(
    struct venu_value *value,
    double floating
)
{
    if (value == NULL) {
        return;
    }

    value->type = VENU_VALUE_FLOAT;
    value->as.floating = floating;
}

bool venu_value_initialize_string(
    struct venu_value *value,
    const char *data,
    size_t length,
    const struct venu_allocator *allocator
)
{
    if (value == NULL ||
        (data == NULL && length != 0) ||
        length == SIZE_MAX ||
        !venu_allocator_is_valid(allocator)) {
        return false;
    }

    char *owned_data = venu_allocate(
        allocator,
        length + 1
    );

    if (owned_data == NULL) {
        return false;
    }

    if (length != 0) {
        memcpy(
            owned_data,
            data,
            length
        );
    }

    owned_data[length] = '\0';

    value->type = VENU_VALUE_STRING;
    value->as.string.data = owned_data;
    value->as.string.length = length;
    value->as.string.owns_data = true;

    return true;
}

bool venu_value_initialize_borrowed_string(
    struct venu_value *value,
    char *data,
    size_t length
)
{
    if (value == NULL ||
        (data == NULL && length != 0)) {
        return false;
    }

    value->type = VENU_VALUE_STRING;
    value->as.string.data = data;
    value->as.string.length = length;
    value->as.string.owns_data = false;

    return true;
}

bool venu_value_clone(
    struct venu_value *destination,
    const struct venu_value *source,
    const struct venu_allocator *allocator
)
{
    if (destination == NULL ||
        source == NULL) {
        return false;
    }

    switch (source->type) {
        case VENU_VALUE_NULL:
            venu_value_initialize_null(destination);
            return true;

        case VENU_VALUE_BOOLEAN:
            venu_value_initialize_boolean(
                destination,
                source->as.boolean
            );
            return true;

        case VENU_VALUE_INTEGER:
            venu_value_initialize_integer(
                destination,
                source->as.integer
            );
            return true;

        case VENU_VALUE_FLOAT:
            venu_value_initialize_float(
                destination,
                source->as.floating
            );
            return true;

        case VENU_VALUE_STRING:
            return venu_value_initialize_string(
                destination,
                source->as.string.data,
                source->as.string.length,
                allocator
            );
    }

    return false;
}

void venu_value_destroy(
    struct venu_value *value,
    const struct venu_allocator *allocator
)
{
    if (value == NULL) {
        return;
    }

    if (value->type == VENU_VALUE_STRING &&
        value->as.string.owns_data) {
        venu_deallocate(
            allocator,
            value->as.string.data
        );
    }

    venu_value_initialize_null(value);
}

const char *venu_value_type_name(
    enum venu_value_type type
)
{
    switch (type) {
        case VENU_VALUE_NULL:
            return "null";

        case VENU_VALUE_BOOLEAN:
            return "boolean";

        case VENU_VALUE_INTEGER:
            return "integer";

        case VENU_VALUE_FLOAT:
            return "float";

        case VENU_VALUE_STRING:
            return "string";
    }

    return "unknown";
}