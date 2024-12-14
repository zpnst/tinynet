#include <stdio.h>
#include <string.h>

#include "src/tinynet.h"


void
panic(char *message)
{
    LOG_PANIC_PREFIX(message);
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

void 
safety_free(void *mem) 
{
    if (mem) {
        free(mem);
    }
}

char *
panic_strdup(char *string)
{
    char *new_char = strdup(string ? string : "");
    if (!new_char) {
        panic("out of memory");
    }
    return new_char;
}