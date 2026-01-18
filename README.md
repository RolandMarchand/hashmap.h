# hashmap.h - Type-Safe Hash Tables for C

A production-ready, macro-based hashmap library to generate type-safe hash tables with separate chaining.

```c
#include "hashmap.h"

/* Arguments: struct name, func. prefix, key type, value type, hash func (opt), compare func (opt) */
HASHMAP_DECLARE(IntMap, int_map, int, int, NULL, NULL)
HASHMAP_DEFINE(IntMap, int_map, int, int, NULL, NULL)

int main(void) {
	IntMap map = { 0 };

	int_map_insert(&map, 10, 42);
	int_map_insert(&map, 20, 7);

	int value;
	if (int_map_get(&map, 10, &value)) {
		printf("Found: %d\n", value);  /* 42 */
	}

	printf("Size: %zu\n", map.size);   /* 2 */

	int_map_free(&map);
}
```

## Features

- **Type-safe**: Generate hashmaps for any key-value pair types
- **Portable**: C89 compatible, tested on GCC/Clang/MSVC/ICX across x86/x86_64/ARM64
- **Flexible**: Custom hash and comparison functions, or use built-in defaults
- **Efficient**: Separate chaining with 0.75 load factor, power-of-2 growth
- **Configurable**: Custom allocators, null-pointer policies
- **Zero dependencies**: Just standard C library

## Quick Start

### For String Keys (Common Case)

1. Include the header and declare your hashmap:

```c
/* my_hashmap.h */
#include "hashmap.h"

/* Automatically uses strcmp and FNV-1a string hash */
HASHMAP_DECLARE_STRING(UserMap, user_map, int)
```

2. Define the implementation (usually in a .c file):

```c
/* my_hashmap.c */
#include "my_hashmap.h"

HASHMAP_DEFINE_STRING(UserMap, user_map, int)
```

3. Use it:

```c
UserMap users = {0};

user_map_insert(&users, "alice", 100);
user_map_insert(&users, "bob", 200);

int score;
if (user_map_get(&users, "alice", &score)) {
	printf("Alice's score: %d\n", score);
}

user_map_remove(&users, "bob", NULL);
user_map_free(&users);
```

### For Custom Key Types

```c
/* my_hashmap.h */
#include "hashmap.h"

unsigned long my_hash_func(int key);
int my_compare_func(int a, int b);

/* Pass NULL for hash/compare to use default FNV-1a/memcmp */
HASHMAP_DECLARE(IntMap, int_map, int, float, my_hash_func, my_compare_func)
```

```c
/* my_hashmap.c */
#include "my_hashmap.h"

HASHMAP_DEFINE(IntMap, int_map, int, float, my_hash_func, my_compare_func)

unsigned long my_hash_func(int key) {
	return (unsigned long)key * 2654435761UL;
}

int my_compare_func(int a, int b) {
	return a - b;
}
```

Alternatively, if you plan to use your hashmap within a single file:

```c
#include "hashmap.h"

HASHMAP_DECLARE_STRING(MyMap, my_map, int)
HASHMAP_DEFINE_STRING(MyMap, my_map, int)
```

## Important: Pointer Keys vs. Content

**By default, keys are compared by value, not content.** If your key is a `char *` and you pass `NULL` for the comparison function, **the pointer addresses will be compared**, not the string contents.

```c
/* WRONG: Compares pointer addresses, not string content */
HASHMAP_DECLARE(BadMap, bad_map, char *, int, NULL, NULL)

/* CORRECT: Uses strcmp and fnv1a_32_str (comes with library) to compare string content */
HASHMAP_DECLARE(GoodMap, good_map, const char *, int, good_map_fnv1a_32_str, strcmp)

/* BEST: For string keys, use the STRING variant */
HASHMAP_DECLARE_STRING(BestMap, best_map, int)
```

## API Overview

- `hashmap_init(map)` - Initializes the hashmap
- `hashmap_grow(map)` - Grow the hashmap
- `hashmap_insert(map, key, value)` - Insert or update (returns 1 if overwritten, 0 if new)
- `hashmap_get(map, key, &out)` - Retrieve value (returns 1 if found, 0 otherwise)
- `hashmap_has(map, key)` - Check if key exists
- `hashmap_size(map)` - Return the amount of elements stored in the hashmap, same as map.size
- `hashmap_remove(map, key, &out)` - Remove key-value pair (returns 1 if removed, 0 if not found)
- `hashmap_iterate(map, context)` - Iterate over all pairs using callback
- `hashmap_duplicate(dest, src)` - Deep copy hashmap
- `hashmap_clear(map)` - Remove all elements (keeps capacity)
- `hashmap_free(map)` - Deallocate memory

## Iteration

Set the `iteration_callback` field and call `hashmap_iterate()`:

```c
int print_pair(const char *key, int value, void *context) {
	printf("%s: %d\n", key, value);
	return 1;  /* Continue iterating (return 0 to stop) */
}

StringMap map = {0};
map.iteration_callback = print_pair;

string_map_insert(&map, "one", 1);
string_map_insert(&map, "two", 2);

string_map_iterate(&map, NULL);  /* Pass context if needed */
```

## Configuration

Define before including the library:

```c
#define HASHMAP_NO_PANIC_ON_NULL 1    /* Return silently on NULL instead of panic */
#define HASHMAP_REALLOC my_realloc    /* Custom allocator */
#define HASHMAP_FREE my_free          /* Custom deallocator */
```

## Testing

```bash
mkdir build
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug
cd build
make test
```

Tests cover insert/remove operations, collision handling, growth, iteration, and edge cases.

## Contribution

Contributors and library hackers should work on `hashmap.in.h` instead of `hashmap.h`. It is a version of the library with hardcoded types and function names. To generate the final library from it, run `libgen.py`.

## License

This library is licensed under the BSD Zero license.
