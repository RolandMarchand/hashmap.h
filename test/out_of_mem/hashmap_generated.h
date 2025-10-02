#ifndef HASHMAP_GENERATED_H
#define HASHMAP_GENERATED_H

#include <stdlib.h>

void *return_null(void *ptr, size_t size);

#define HASHMAP_REALLOC(p, s) (return_null((p), (s)))
#define HASHMAP_FREE(p) (free((p)))
#define HASHMAP_LONG_JUMP_NO_ABORT

#include "hashmap.h"

#endif /* HASHMAP_GENERATED_H */

