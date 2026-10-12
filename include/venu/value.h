#ifndef VENU_VALUE_H
#define VENU_VALUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "venu/allocator.h"

enum venu_value_type {
    VENU_VALUE_NULL,
    VENU_VALUE_BOOLEAN,
    VENU_VALUE_INTEGER,
    VENU_VALUE_FLOAT,
    VENU_VALUE_STRING
};

struct venu_string {
    /*
     * Strings are always null-terminated.
     * length does not include the terminating null byte.
     */
    char *data;
    size_t length;

    /*
     * true:
     *     data was allocated through a VENU allocator and must
     *     eventually be released.
     *
     * false:
     *     another object, such as the parser arena, owns data.
     */
    bool owns_data;
};

struct venu_value {
    enum venu_value_type type;

    union {
        bool boolean;
        int64_t integer;
        double floating;
        struct venu_string string;
    } as;
};

void venu_value_initialize_null(
    struct venu_value *value
);

void venu_value_initialize_boolean(
    struct venu_value *value,
    bool boolean
);

void venu_value_initialize_integer(
    struct venu_value *value,
    int64_t integer
);

void venu_value_initialize_float(
    struct venu_value *value,
    double floating
);

bool venu_value_initialize_string(
    struct venu_value *value,
    const char *data,
    size_t length,
    const struct venu_allocator *allocator
);

bool venu_value_initialize_borrowed_string(
    struct venu_value *value,
    char *data,
    size_t length
);

bool venu_value_clone(
    struct venu_value *destination,
    const struct venu_value *source,
    const struct venu_allocator *allocator
);

void venu_value_destroy(
    struct venu_value *value,
    const struct venu_allocator *allocator
);

const char *venu_value_type_name(
    enum venu_value_type type
);

#endif