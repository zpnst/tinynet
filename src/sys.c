#include <stdio.h>
#include <string.h>

#include "src/tinynet.h"

void
panic(tinynet_char_t *message)
{
    fprintf(stderr, "[PANIC] %s\n", message);
    exit(1);
}

void *
panic_alloc(size_t size)
{
    void *new_prt = calloc(1, size);
    if (!new_prt) {
        panic("out of memory");
    }
    return new_prt;
}

tinynet_char_t *
panic_strdup(tinynet_char_t *string)
{
    tinynet_char_t *new_char = strdup(string ? string : "");
    if (!new_char) {
        panic("out of memory");
    }
    return new_char;
}