#ifndef SYS_H
#define SYS_H

#include "src/tinynet.h"

void
panic(tinynet_char_t *message);

void *
panic_alloc(size_t size);

tinynet_char_t *
panic_strdup(tinynet_char_t *string);

#endif /** SYS_H */