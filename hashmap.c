#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"

#define FNV1A_32_PRIME 0x01000193U
#define FNV1A_32_INITIAL_VAL 0x811c9dc5U

unsigned long fnv1a_32_str(const char *str)
{
	const unsigned char *ustr = (const unsigned char *)str;
	unsigned long hval = FNV1A_32_INITIAL_VAL;

	for (; ustr[0] != '\0'; ustr++) {
		hval ^= (unsigned long)ustr[0];
		hval *= FNV1A_32_PRIME;
		hval &= 0xFFFFFFFFUL;
	}

	return hval;
}

int free_callback(const char *key, int value, void *context)
{
	(void)context;
	/* Nothing to do for keys because they're string literals */
	/* Nothing to do for values because integers are only passed by value */
	printf("freeing %s : %d\n", key, value);
	return 0; /* continue, !0 means stop */
}

int main(void)
{
	struct Hashmap map = { 0 };
	int exists = 0;
	int existed = 0;
	int override = 0;
	int stored = 0;
	int removed = 0;

	hashmap_init(&map);

	/* both passed by value, pointers are copied if there are any */
	override = hashmap_insert(&map, "hello", 10);
	assert(!override);

	assert(map.size == 1);
	assert(map.capacity == 8); /* Default capcity of 8, must be a power of 2, 75% load factor */

	/* Hash and comparison callbacks called on copy of "hello" pointer */
	/* Results stored in "store" if the value exists */
	exists = hashmap_get(&map, "hello", &stored);
	assert(exists);
	assert(stored == 10);

	existed = hashmap_remove(&map, "hello", &removed);
	assert(existed);
	assert(removed == 10);

	exists = hashmap_get(&map, "hello", &stored);
	assert(!exists);

	/* call "hm.iteration_callback() on each key/value pairs" with no
	 * context */
	map.iteration_callback = free_callback;
	hashmap_iterate(&map, NULL);
 	hashmap_free(&map);

	return 0;
}
