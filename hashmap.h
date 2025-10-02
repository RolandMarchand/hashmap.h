/* simple_hashmap.h - A basic hashmap implementation for C89 */
#ifndef _SIMPLE_HASHMAP_H
#define _SIMPLE_HASHMAP_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* My dream hashmap:
 * - Let's start with only char* -> int
 * - separate chaining with linked list
 * - linked list has heap-allocated nodes
 * - 70% growth trigger
 * - 2x growth factor
 * - x & (n - 1) instead of %
 * - panic on error
 * - c89 standard
 * - no small size optimization
 * - scattered nodes via dynamic allocation
 * - pass equality function (start with strcmp)
 * - pass hash function (start with fnv1a_32_str) */

#if defined(HASHMAP_REALLOC) && !defined(HASHMAP_FREE) || \
	!defined(HASHMAP_REALLOC) && defined(HASHMAP_FREE)
#error "You must define both HASHMAP_REALLOC and HASHMAP_FREE, or neither."
#endif
#if !defined(HASHMAP_REALLOC) && !defined(HASHMAP_FREE)
#define HASHMAP_REALLOC(p, s) (realloc((p), (s)))
#define HASHMAP_FREE(p) (free((p)))
#endif

#ifndef HASHMAP_NO_PANIC_ON_NULL
#define HASHMAP_NO_PANIC_ON_NULL 0
#endif

#ifdef HASHMAP_LONG_JUMP_NO_ABORT
#include <setjmp.h>
extern jmp_buf abort_jmp;
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define HASHMAP_NORETURN [[noreturn]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define HASHMAP_NORETURN _Noreturn
#elif defined(__GNUC__) || defined(__clang__)
#define HASHMAP_NORETURN __attribute__((noreturn))
#else
#define HASHMAP_NORETURN
#endif

#ifndef __STDC_VERSION__
#define HASHMAP_INLINE
#elif _MSC_VER
#define HASHMAP_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define HASHMAP_INLINE __attribute__((always_inline)) inline
#else
#define HASHMAP_INLINE inline
#endif

#ifndef __STDC_VERSION__
#define RESTRICT
#else
#define RESTRICT restrict
#endif

/* TODO: to change to a macro-based API */
#ifndef HASH_CALLBACK
#define HASH_CALLBACK NULL
#endif

#ifndef COMPARISON_CALLBACK
#define COMPARISON_CALLBACK NULL
#endif

#define HASHMAP_LOAD_FACTOR 0.75f
enum { HASHMAP_DEFAULT_CAPACITY = 8, HASHMAP_GROWTH_FACTOR = 2 };

static unsigned long hashmap_fnv1a_32_buf(const void *buf, size_t len)
{
	const unsigned char *bptr = (const unsigned char *)buf;
	const unsigned char *bend = bptr + len;
	unsigned long hval = 0x811c9dc5U;

	for (; bptr < bend; bptr++) {
		hval ^= (unsigned long)bptr[0];
		hval *= 0x01000193U;
		hval &= 0xFFFFFFFFUL;
	}

	return hval;
}

typedef int CustomValue;
typedef const char *CustomKey;

/* Declarations start here */

struct HashmapListNode {
	struct HashmapListNode *next;
	CustomKey key;
	CustomValue value;
};

typedef struct Hashmap {
	struct HashmapListNode **buckets;
	int (*iteration_callback)(CustomKey key, CustomValue value,
				  void *context);
	size_t size;
	size_t capacity;
	size_t buckets_filled;
} Hashmap;

/* API functions */
void hashmap_init(Hashmap *map);
void hashmap_grow(Hashmap *map);
int hashmap_insert(Hashmap *map, CustomKey key, CustomValue value);
int hashmap_remove(Hashmap *RESTRICT map, CustomKey key,
		   CustomValue *RESTRICT out);
int hashmap_get(const Hashmap *RESTRICT map, CustomKey key,
		CustomValue *RESTRICT out);
HASHMAP_INLINE int hashmap_has(const Hashmap *map, CustomKey key);
void hashmap_free(Hashmap *map);
void hashmap_iterate(Hashmap *map, void *context);
void hashmap_duplicate(Hashmap *dest, Hashmap *src);
void hashmap_clear(Hashmap *map);

/* Internal functions */
HASHMAP_INLINE void hashmap_assert(const Hashmap *map);
struct HashmapListNode *hashmap_list_new(struct HashmapListNode *next,
					 CustomKey key, CustomValue value);
HASHMAP_INLINE int (*hashmap_compare_comparison_callback(void))(CustomKey,
								CustomKey);
int hashmap_grow_internal(CustomKey key, CustomValue value, void *new_map);
HASHMAP_INLINE int hashmap_compare_keys(CustomKey key1, CustomKey key2);
int hashmap_list_insert(struct HashmapListNode *head, CustomKey key,
			CustomValue value);
int hashmap_list_find(struct HashmapListNode *RESTRICT head, CustomKey key,
		      CustomValue *RESTRICT out);
int hashmap_list_remove(struct HashmapListNode **RESTRICT list, CustomKey key,
			CustomValue *RESTRICT out);
int hashmap_list_iterate(struct HashmapListNode *head,
			 int (*callback)(CustomKey key, CustomValue value,
					 void *context),
			 void *context);
void hashmap_list_free(struct HashmapListNode *head);
struct HashmapListNode *hashmap_list_duplicate(struct HashmapListNode *head);
HASHMAP_INLINE unsigned long (*hashmap_compare_hash_callback(void))(CustomKey);
HASHMAP_INLINE size_t hashmap_hash_index(const Hashmap *map, CustomKey key);
/* Declarations stop here */

#ifdef HASHMAP_LONG_JUMP_NO_ABORT
#define HASHMAP_DEFINE_PANIC(Function_Prefix_)                              \
	HASHMAP_NORETURN void Function_Prefix_##_panic(const char *message) \
	{                                                                   \
		assert(message);                                            \
		longjmp(abort_jmp, 1);                                      \
	}
#else
#define HASHMAP_DEFINE_PANIC(Function_Prefix_)                              \
	HASHMAP_NORETURN void Function_Prefix_##_panic(const char *message) \
	{                                                                   \
		assert(message);                                            \
		(void)fprintf(stderr, "%s\n", message);                     \
		abort();                                                    \
	}
#endif

/* Definitions start here */
struct Hashmap;
struct HashmapListNode;
HASHMAP_DEFINE_PANIC(hashmap)

HASHMAP_INLINE void hashmap_assert(const struct Hashmap *map)
{
	/* If buckets is NULL, map should be in initial/freed state */
	if (map->buckets == NULL) {
		assert(map->size == 0);
		assert(map->buckets_filled == 0);
		assert(map->capacity == 0);
		return;
	}

	/* Capacity must be a power of 2 and non-zero */
	assert(map->capacity > 0);
	assert((map->capacity & (map->capacity - 1)) == 0);

	/* Size invariants */
	assert(map->size >= map->buckets_filled);
	assert(map->buckets_filled <= map->capacity);
}

struct HashmapListNode *hashmap_list_new(struct HashmapListNode *next,
					 CustomKey key, CustomValue value)
{
	struct HashmapListNode *ret = (struct HashmapListNode *)HASHMAP_REALLOC(
		NULL, sizeof(struct HashmapListNode));
	if (ret == NULL) {
		hashmap_panic("Out of memory. Panic.");
	}

	ret->next = next;
	ret->key = key;
	ret->value = value;

	return ret;
}

void hashmap_init(struct Hashmap *map)
{
	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_init but non-null argument expected.");
	}

	memset((void *)map, 0, sizeof(struct Hashmap));

	map->buckets = (struct HashmapListNode **)HASHMAP_REALLOC(
		NULL,
		HASHMAP_DEFAULT_CAPACITY * sizeof(struct HashmapListNode *));

	if (map->buckets == NULL) {
		hashmap_panic("Out of memory. Panic.");
	}

	map->capacity = HASHMAP_DEFAULT_CAPACITY;
	assert(map->capacity > 0);

	memset((void *)map->buckets, 0,
	       map->capacity * sizeof(struct HashmapListNode *));

	hashmap_assert(map);
}

HASHMAP_INLINE int (*hashmap_compare_comparison_callback(void))(CustomKey,
								CustomKey)
{
	return COMPARISON_CALLBACK;
}

HASHMAP_INLINE int hashmap_compare_keys(CustomKey key1, CustomKey key2)
{
	int (*callback)(CustomKey, CustomKey) =
		hashmap_compare_comparison_callback();

	if (callback == NULL) {
		return memcmp((void *)&key1, (void *)&key2, sizeof(CustomKey));
	}
	return callback(key1, key2);
}

/* Assume the first node isn't NULL */
int hashmap_list_insert(struct HashmapListNode *head, CustomKey key,
			CustomValue value)
{
	struct HashmapListNode *prev = NULL;
	size_t iter = 0;

	assert(head != NULL);

	for (; head != NULL; prev = head, head = head->next, iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);

		if (hashmap_compare_keys(key, head->key) == 0) {
			/* Override existing value */
			head->value = value;
			return 1;
		}
	}

	/* Append to end of list */
	prev->next = hashmap_list_new(NULL, key, value);
	return 0;
}

int hashmap_list_find(struct HashmapListNode *RESTRICT head, CustomKey key,
		      CustomValue *RESTRICT out)
{
	size_t iter = 0;

	for (iter = 0; head != NULL; head = head->next, iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);

		if (hashmap_compare_keys(head->key, key) == 0) {
			if (out != NULL) {
				*out = head->value;
			}
			return 1;
		}
	}

	return 0;
}

int hashmap_list_remove(struct HashmapListNode **RESTRICT list, CustomKey key,
			CustomValue *RESTRICT out)
{
	struct HashmapListNode *head = NULL;
	struct HashmapListNode *prev = NULL;
	size_t iter = 0;

	if (list == NULL) {
		return 0;
	}

	head = *list;

	for (iter = 0; head != NULL; prev = head, head = head->next, iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);

		if (hashmap_compare_keys(head->key, key) != 0) {
			continue;
		}

		if (out != NULL) {
			*out = head->value;
		}

		if (prev == NULL) {
			*list = head->next;
		} else {
			prev->next = head->next;
		}

		HASHMAP_FREE(head);
		return 1;
	}

	return 0;
}

int hashmap_list_iterate(struct HashmapListNode *head,
			 int (*callback)(CustomKey key, CustomValue value,
					 void *context),
			 void *context)
{
	size_t iter = 0;

	assert(callback != NULL);

	for (iter = 0; head != NULL; head = head->next, iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);

		if (callback(head->key, head->value, context) == 0) {
			return 0;
		}
	}

	return 1;
}

void hashmap_list_free(struct HashmapListNode *head)
{
	struct HashmapListNode *next = NULL;
	size_t iter = 0;

	for (iter = 0; head != NULL; iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);

		next = head->next;
		HASHMAP_FREE(head);
		head = next;
	}
}

struct HashmapListNode *hashmap_list_duplicate(struct HashmapListNode *head)
{
	struct HashmapListNode *new_head = NULL;
	struct HashmapListNode *new_next = NULL;
	size_t iter = 0;

	if (head == NULL) {
		return NULL;
	}

	new_head = hashmap_list_new(NULL, head->key, head->value);
	new_next = new_head;

	for (iter = 0; head->next != NULL;
	     head = head->next, new_next = new_next->next, iter++) {
		/* Debug test for infinite loops */
		assert(iter < 0xFFFFFFFFUL);
		new_next->next = hashmap_list_new(NULL, head->next->key,
						  head->next->value);
	}

	return new_head;
}

HASHMAP_INLINE unsigned long (*hashmap_compare_hash_callback(void))(CustomKey)
{
	return HASH_CALLBACK;
}

HASHMAP_INLINE size_t hashmap_hash_index(const struct Hashmap *map,
					 CustomKey key)
{
	size_t idx = 0;
	unsigned long (*callback)(CustomKey) = hashmap_compare_hash_callback();

	if (callback == NULL) {
		idx = hashmap_fnv1a_32_buf((const void *)&key,
					   sizeof(CustomKey));
	} else {
		idx = callback(key);
	}

	idx &= map->capacity - 1;

	return idx;
}

void hashmap_grow(struct Hashmap *map)
{
	size_t new_capacity = 0;
	struct Hashmap new_map = { 0 };
	int (*iteration_callback_backup)(CustomKey, CustomValue, void *);
	size_t new_capacity_size = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_grow but non-null argument expected.");
	}

	hashmap_assert(map);

	if (map->buckets == NULL) {
		hashmap_init(map);
	}

	iteration_callback_backup = map->iteration_callback;

	/* Calculate the next power of 2 */
	new_capacity = map->capacity;
	new_capacity |= new_capacity >> 1;
	new_capacity |= new_capacity >> 2;
	new_capacity |= new_capacity >> 4;
	new_capacity |= new_capacity >> 8;
	if (sizeof(size_t) >= 4) {
		new_capacity |= new_capacity >> 16;
	}
	if (sizeof(size_t) >= 8) {
		new_capacity |= new_capacity >> 32;
	}
	new_capacity++;

	if (new_capacity < map->capacity) {
		/* Overflow, do not grow, let separate chaining handle
		 * collisions */
		return;
	}
	if (new_capacity > ((size_t)-1) /  sizeof(struct HashmapListNode *)) {
		/* Would overflow, do not grow, let separate chaining handle
		 * collisions */
		return;		
	}

	new_capacity_size = new_capacity * sizeof(struct HashmapListNode *);
	new_map.capacity = new_capacity;
	new_map.buckets =
		(struct HashmapListNode **)HASHMAP_REALLOC(NULL, new_capacity_size);
	if (new_map.buckets == NULL) {
		hashmap_panic("Out of memory. Panic.");
	}
	memset((void *)new_map.buckets, 0, new_capacity_size);

	map->iteration_callback = hashmap_grow_internal;
	hashmap_iterate(map, &new_map);

	assert(map->size >= new_map.size);
	if (map->size != new_map.size) {
		/* Failed to grow, revert operation */
		map->iteration_callback = iteration_callback_backup;
		hashmap_free(&new_map);
		return;
	}

	hashmap_free(map);
	*map = new_map;
	map->iteration_callback = iteration_callback_backup;
}

int hashmap_grow_internal(CustomKey key, CustomValue value, void *new_map)
{
	/* Should never overwrite */
	assert(hashmap_insert(new_map, key, value) == 0);
	return 1;
}

int hashmap_insert(struct Hashmap *map, CustomKey key, CustomValue value)
{
	size_t idx = 0;
	int overwritten = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return -1;
		}
		hashmap_panic(
			"Null passed to hashmap_insert but non-null argument expected.");
	}

	hashmap_assert(map);

	if (map->buckets == NULL) {
		hashmap_init(map);
	}

	/* Check for being above load factor */
	if ((float)map->buckets_filled / (float)map->capacity >
	    HASHMAP_LOAD_FACTOR) {
		hashmap_grow(map);
	}

	idx = hashmap_hash_index(map, key);

	if (map->buckets[idx] == NULL) {
		map->buckets[idx] = hashmap_list_new(NULL, key, value);
		map->buckets_filled++;
		map->size++;
		return 0;
	}

	overwritten = hashmap_list_insert(map->buckets[idx], key, value);
	if (!overwritten) {
		map->size++;
	}

	return overwritten;
}

int hashmap_remove(struct Hashmap *RESTRICT map, CustomKey key,
		   CustomValue *RESTRICT out)
{
	size_t idx = 0;
	int found = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return -1;
		}
		hashmap_panic(
			"Null passed to hashmap_remove but non-null argument expected.");
	}

	hashmap_assert(map);

	if (map->buckets == NULL) {
		return 0;
	}

	idx = hashmap_hash_index(map, key);

	found = hashmap_list_remove(&map->buckets[idx], key, out);

	if (found) {
		if (map->buckets[idx] == NULL) {
			assert(map->buckets_filled > 0);
			map->buckets_filled--;
		}
		assert(map->size > 0);
		map->size--;
	}

	return found;
}

int hashmap_get(const struct Hashmap *RESTRICT map, CustomKey key,
		CustomValue *RESTRICT out)
{
	size_t idx = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return -1;
		}
		hashmap_panic(
			"Null passed to hashmap_get but non-null argument expected.");
	}

	hashmap_assert(map);

	if (map->buckets == NULL) {
		return 0;
	}

	idx = hashmap_hash_index(map, key);

	return hashmap_list_find(map->buckets[idx], key, out);
}

HASHMAP_INLINE int hashmap_has(const struct Hashmap *map, CustomKey key)
{
	return hashmap_get(map, key, NULL);
}

void hashmap_free(struct Hashmap *map)
{
	size_t idx = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_free but non-null argument expected.");
	}

	hashmap_assert(map);

	for (idx = 0; idx < map->capacity; idx++) {
		hashmap_list_free(map->buckets[idx]);
	}

	HASHMAP_FREE((void *)map->buckets);

	memset((void *)map, 0, sizeof(struct Hashmap));
}

void hashmap_iterate(struct Hashmap *map, void *context)
{
	size_t idx = 0;
	int callback_response = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_iterate but non-null argument expected.");
	}

	hashmap_assert(map);

	if (map->iteration_callback == NULL) {
		return;
	}

	for (idx = 0; idx < map->capacity; idx++) {
		callback_response = hashmap_list_iterate(
			map->buckets[idx], map->iteration_callback, context);
		if (callback_response == 0) {
			break;
		}
	}
}

void hashmap_duplicate(struct Hashmap *dest, struct Hashmap *src)
{
	size_t idx = 0;

	if (dest == NULL || src == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_clear but non-null argument expected.");
	}
	hashmap_assert(src);

	if (src->capacity == 0) {
		memset(dest, 0, sizeof(struct Hashmap));
		return;
	}

	dest->buckets = (struct HashmapListNode **)HASHMAP_REALLOC(
		NULL, src->capacity * sizeof(struct HashmapListNode *));

	if (dest->buckets == NULL) {
		hashmap_panic("Out of memory. Panic.");
	}

	dest->capacity = src->capacity;
	dest->size = src->size;
	dest->buckets_filled = src->buckets_filled;
	dest->iteration_callback = src->iteration_callback;

	memset((void *)dest->buckets, 0,
	       dest->capacity * sizeof(struct HashmapListNode *));

	for (idx = 0; idx < dest->capacity; idx++) {
		dest->buckets[idx] = hashmap_list_duplicate(src->buckets[idx]);
	}

	hashmap_assert(dest);
}

void hashmap_clear(struct Hashmap *map)
{
	size_t idx = 0;

	if (map == NULL) {
		if (HASHMAP_NO_PANIC_ON_NULL) {
			return;
		}
		hashmap_panic(
			"Null passed to hashmap_clear but non-null argument expected.");
	}

	hashmap_assert(map);

	for (idx = 0; idx < map->capacity; idx++) {
		hashmap_list_free(map->buckets[idx]);
		map->buckets[idx] = NULL;
	}

	map->size = 0;
	map->buckets_filled = 0;
}

/* Definitions stop here */

/****************************************************************************
 * Copyright (C) 2025 by Roland Marchand <roland.marchand@protonmail.com>   *
 *                                                                          *
 * Permission to use, copy, modify, and/or distribute this software for any *
 * purpose with or without fee is hereby granted.                           *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************/

#endif /* _SIMPLE_HASHMAP_H */
