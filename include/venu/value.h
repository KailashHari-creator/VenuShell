#ifndef VENU_VALUE_H
#define VENU_VALUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#endif