#ifndef SYS_H
#define SYS_H

#include "src/tinynet.h"

void
panic(char *message);

void *
panic_alloc(size_t size);

void 
safety_free(void *mem);

char *
panic_strdup(char *string);

#endif /** SYS_H */