/* simple_hashmap.h - A basic hashmap implementation for C89 */
#ifndef _SIMPLE_HASHMAP_H
#define _SIMPLE_HASHMAP_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Library to generate type-safe hashmap types.
 * 
 * This library is portable (tested on GCC/Clang/MSVC/ICX, x86/x86_64/ARM64,
 * all warnings and pedantic) and is C89 compatible.
 *
 * To generate hashmaps, use the macros HASHMAP_DECLARE() to generate the
 * header, and HASHMAP_DEFINE() to generate the source. For string hashmaps, use
 * the macros HASHMAP_DECLARE_STRING() and HASHMAP_DEFINE_STRING(). It is
 * recommended to place them in their respective files. Generate as many
 * different types of hashmaps as you want.
 *
 * HASHMAP_DECLARE() takes six arguments: the struct name, the function prefix,
 * the key type, the value type, an optional hash function (NULL for default
 * FNV-1a), and an optional key comparison function (NULL for memcmp).
 *
 * HASHMAP_DECLARE_STRING() takes three arguments: the struct name, the function
 * prefix, and the value type. The key type is automatically set to const char *,
 * the hash function to FNV-1a (reads string content instead of raw pointer),
 * and the comparison function to strcmp.
 *
 * Note that with HASHMAP_DECLARE() and HASHMAP_DEFINE() by default, the key
 * value itself is compared and hashed. For instance, if the key is a char * and
 * the hash and comparison functions are set to NULL, the pointer itself will be
 * hashed and compared (not the string it points to).
 *
 * This means two different strings with the same contents won't be equal or
 * produce the same hash. To fix this, pass hashing and comparison functions
 * that read the pointer's content, or use HASHMAP_DECLARE_STRING() and
 * HASHMAP_DEFINE_STRING() if your keys are const char *.
 *
 * This library is not thread safe.
 *
 * This library uses separate chaining with linked lists for collision
 * resolution.
 *
 * Load factor is set at 0.75, with capacity growing by powers of 2.
 * 
 * To configure this library #define the symbols before including the library.
 * This is usually done in the header file where HASHMAP_DECLARE() is called.
 * 
 * Configuration options:
 * 
 * - HASHMAP_NO_PANIC_ON_NULL (default 0): if true (1), does not panic upon
 *   passing NULL to hashmap functions. Otherwise, panic.
 * 
 * - HASHMAP_REALLOC (default realloc(3)): specify the allocator. If using
 *   a custom allocator, must also specify HASHMAP_FREE.
 * 
 * - HASHMAP_FREE (default free(3)): specify the deallocator. If using a
 *   custom deallocator, must also specify HASHMAP_REALLOC.
 * 
 * - HASHMAP_LONG_JUMP_NO_ABORT (default undefined): for testing only. Jump to
 *   externally defined "jmp_buf abort_jmp" instead of panicking. Unlike other
 *   configuration options, must be defined before including the library.
 *
 * 
 * API Functions:
 *
 * The following documentation takes this generated hashmap for instance:
 * HASHMAP_DECLARE(Hashmap, hashmap, const char *, int, fnv1a_32_str, strcmp)
 * 
 * All functions panic if map is NULL (unless HASHMAP_NO_PANIC_ON_NULL).
 *
 * void hashmap_init(Hashmap *map)
 *   Initialize empty hashmap with default capacity. Leaks memory if initializes
 *   an already initialized hashmap.
 *
 * void hashmap_free(Hashmap *map)
 *   Deallocate hashmap memory. Safe to call on already-freed hashmaps.
 *
 * void hashmap_grow(Hashmap *map)
 *   Increase capacity to next power of 2. Auto-initializes empty hashmaps.
 *   No-op if growth would cause overflow.
 *
 * int hashmap_insert(Hashmap *map, const char *key, int value)
 *   Insert or update key-value pair. Auto-initializes empty hashmaps.
 *   Grows capacity if load factor exceeds 0.75. Returns 1 if key existed
 *   and value was overwritten, 0 if new key was inserted.
 *
 * int hashmap_remove(Hashmap *map, const char *key, int *out)
 *   Remove key-value pair. If out is non-NULL, stores removed value.
 *   Returns 1 if key was found and removed, 0 otherwise.
 *
 * int hashmap_get(const Hashmap *map, const char *key, int *out)
 *   Get value for key. If out is non-NULL, stores value.
 *   Returns 1 if key found, 0 otherwise.
 *
 * int hashmap_has(const Hashmap *map, const char *key)
 *   Check if key exists in hashmap. Returns 1 if found, 0 otherwise.
 *
 * void hashmap_iterate(Hashmap *map, void *context)
 *   Iterate over all key-value pairs using map->iteration_callback.
 *   Callback signature: int callback(const char *key, int value, void *context).
 *   Callback should return 1 to continue iteration, 0 to stop.
 *   No-op if iteration_callback is NULL.
 *
 * void hashmap_duplicate(Hashmap *dest, const Hashmap *src)
 *   Copy src hashmap to dest. dest must be uninitialized. Overwrites existing
 *   dest data without freeing it.
 *
 * void hashmap_clear(Hashmap *map)
 *   Remove all elements without deallocating capacity. Safe to call on
 *   uninitialized hashmaps.
 *
 *
 * Example:
 *  int main(void)
 *  {
 *     Hashmap map = {0};
 *     int value;
 *     
 *     hashmap_insert(&map, "hello", 10);
 *     hashmap_insert(&map, "world", 20);
 *     hashmap_insert(&map, "test", 30);
 *     
 *     if (hashmap_get(&map, "hello", &value)) {
 *         printf("Found: %d\n", value);
 *     }
 *     
 *     if (hashmap_has(&map, "world")) {
 *         hashmap_remove(&map, "world", NULL);
 *     }
 *     
 *     hashmap_insert(&map, "hello", 99);  // Overwrites existing value
 *     
 *     hashmap_clear(&map);
 *     
 *     hashmap_free(&map);
 *     
 *     return 0;
 *  }
 */

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



#define HASHMAP_LOAD_FACTOR 0.75f
enum { HASHMAP_DEFAULT_CAPACITY = 8, HASHMAP_GROWTH_FACTOR = 2 };


#define HASHMAP_DECLARE_STRING(Struct_Name_, Functions_Prefix_,               \
			       Custom_Value_Type_)                            \
	HASHMAP_DECLARE(Struct_Name_, Functions_Prefix_, const char *,	      \
			Custom_Value_Type_, Functions_Prefix_##_fnv1a_32_str, \
			strcmp)

#define HASHMAP_DEFINE_STRING(Struct_Name_, Functions_Prefix_,               \
			      Custom_Value_Type_)                            \
	HASHMAP_DEFINE(Struct_Name_, Functions_Prefix_, const char *,	     \
		       Custom_Value_Type_, Functions_Prefix_##_fnv1a_32_str, \
		       strcmp)

#define HASHMAP_DECLARE(Struct_Name_, Functions_Prefix_, Custom_Key_Type_, Custom_Value_Type_, Custom_Hash_Func_, Custom_Comparison_Func_)\
\
struct Struct_Name_##ListNode {\
	struct Struct_Name_##ListNode *next;\
	Custom_Key_Type_ key;\
	Custom_Value_Type_ value;\
};\
\
typedef struct Struct_Name_ {\
	struct Struct_Name_##ListNode **buckets;\
	int (*iteration_callback)(Custom_Key_Type_ key, Custom_Value_Type_ value,\
				  void *context);\
	size_t size;\
	size_t capacity;\
	size_t buckets_filled;\
} Struct_Name_;\
\
/* API functions */\
void Functions_Prefix_##_init(Struct_Name_ *map);\
void Functions_Prefix_##_grow(Struct_Name_ *map);\
int Functions_Prefix_##_insert(Struct_Name_ *map, Custom_Key_Type_ key, Custom_Value_Type_ value);\
int Functions_Prefix_##_remove(Struct_Name_ *RESTRICT map, Custom_Key_Type_ key,\
		   Custom_Value_Type_ *RESTRICT out);\
int Functions_Prefix_##_get(const Struct_Name_ *RESTRICT map, Custom_Key_Type_ key,\
		Custom_Value_Type_ *RESTRICT out);\
HASHMAP_INLINE int Functions_Prefix_##_has(const Struct_Name_ *map, Custom_Key_Type_ key);\
void Functions_Prefix_##_free(Struct_Name_ *map);\
void Functions_Prefix_##_iterate(Struct_Name_ *map, void *context);\
void Functions_Prefix_##_duplicate(Struct_Name_ *dest, Struct_Name_ *src);\
void Functions_Prefix_##_clear(Struct_Name_ *map);\
\
/* Internal functions */\
HASHMAP_INLINE void Functions_Prefix_##_assert(const Struct_Name_ *map);\
HASHMAP_INLINE void Functions_Prefix_##_assert_internal(const struct Struct_Name_ *map);\
struct Struct_Name_##ListNode *Functions_Prefix_##_list_new(struct Struct_Name_##ListNode *next,\
					 Custom_Key_Type_ key, Custom_Value_Type_ value);\
HASHMAP_INLINE int (*Functions_Prefix_##_compare_comparison_callback(void))(Custom_Key_Type_,\
								Custom_Key_Type_);\
int Functions_Prefix_##_grow_internal(Custom_Key_Type_ key, Custom_Value_Type_ value, void *new_map);\
HASHMAP_INLINE int Functions_Prefix_##_compare_keys(Custom_Key_Type_ key1, Custom_Key_Type_ key2);\
int Functions_Prefix_##_list_insert(struct Struct_Name_##ListNode *head, Custom_Key_Type_ key,\
			Custom_Value_Type_ value);\
int Functions_Prefix_##_list_find(struct Struct_Name_##ListNode *RESTRICT head, Custom_Key_Type_ key,\
		      Custom_Value_Type_ *RESTRICT out);\
int Functions_Prefix_##_list_remove(struct Struct_Name_##ListNode **RESTRICT list, Custom_Key_Type_ key,\
			Custom_Value_Type_ *RESTRICT out);\
int Functions_Prefix_##_list_iterate(struct Struct_Name_##ListNode *head,\
			 int (*callback)(Custom_Key_Type_ key, Custom_Value_Type_ value,\
					 void *context),\
			 void *context);\
void Functions_Prefix_##_list_free(struct Struct_Name_##ListNode *head);\
struct Struct_Name_##ListNode *Functions_Prefix_##_list_duplicate(struct Struct_Name_##ListNode *head);\
HASHMAP_INLINE unsigned long (*Functions_Prefix_##_compare_hash_callback(void))(Custom_Key_Type_);\
HASHMAP_INLINE size_t Functions_Prefix_##_hash_index(const Struct_Name_ *map, Custom_Key_Type_ key);\
unsigned long Functions_Prefix_##_fnv1a_32_buf(const void *buf, size_t len);\
unsigned long Functions_Prefix_##_fnv1a_32_str(const char *str);

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

#define HASHMAP_DEFINE(Struct_Name_, Functions_Prefix_, Custom_Key_Type_, Custom_Value_Type_, Custom_Hash_Func_, Custom_Comparison_Func_)\
struct Struct_Name_;\
struct Struct_Name_##ListNode;\
HASHMAP_DEFINE_PANIC(Functions_Prefix_)\
\
HASHMAP_INLINE void Functions_Prefix_##_assert(const struct Struct_Name_ *map)\
{\
	/* If buckets is NULL, map should be in initial/freed state */\
	if (map->buckets == NULL) {\
		assert(map->size == 0);\
		assert(map->buckets_filled == 0);\
		assert(map->capacity == 0);\
		return;\
	}\
\
	Functions_Prefix_##_assert_internal(map);\
}\
\
/* Separated into its own function to avoid Clang complexity warning */\
HASHMAP_INLINE void Functions_Prefix_##_assert_internal(const struct Struct_Name_ *map)\
{\
	/* Capacity must be a power of 2 and non-zero */\
	assert(map->capacity > 0);\
	assert((map->capacity & (map->capacity - 1)) == 0);\
\
	/* Size invariants */\
	assert(map->size >= map->buckets_filled);\
	assert(map->buckets_filled <= map->capacity);\
}\
\
struct Struct_Name_##ListNode *Functions_Prefix_##_list_new(struct Struct_Name_##ListNode *next,\
					 Custom_Key_Type_ key, Custom_Value_Type_ value)\
{\
	struct Struct_Name_##ListNode *ret = (struct Struct_Name_##ListNode *)HASHMAP_REALLOC(\
		NULL, sizeof(struct Struct_Name_##ListNode));\
	if (ret == NULL) {\
		Functions_Prefix_##_panic("Out of memory. Panic.");\
	}\
\
	ret->next = next;\
	ret->key = key;\
	ret->value = value;\
\
	return ret;\
}\
\
void Functions_Prefix_##_init(struct Struct_Name_ *map)\
{\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_init but non-null argument expected.");\
	}\
\
	memset((void *)map, 0, sizeof(struct Struct_Name_));\
\
	map->buckets = (struct Struct_Name_##ListNode **)HASHMAP_REALLOC(\
		NULL,\
		HASHMAP_DEFAULT_CAPACITY * sizeof(struct Struct_Name_##ListNode *));\
\
	if (map->buckets == NULL) {\
		Functions_Prefix_##_panic("Out of memory. Panic.");\
	}\
\
	map->capacity = HASHMAP_DEFAULT_CAPACITY;\
	assert(map->capacity > 0);\
\
	memset((void *)map->buckets, 0,\
	       map->capacity * sizeof(struct Struct_Name_##ListNode *));\
\
	Functions_Prefix_##_assert(map);\
}\
\
HASHMAP_INLINE int (*Functions_Prefix_##_compare_comparison_callback(void))(Custom_Key_Type_,\
								Custom_Key_Type_)\
{\
	return Custom_Comparison_Func_;\
}\
\
HASHMAP_INLINE int Functions_Prefix_##_compare_keys(Custom_Key_Type_ key1, Custom_Key_Type_ key2)\
{\
	int (*callback)(Custom_Key_Type_, Custom_Key_Type_) =\
		Functions_Prefix_##_compare_comparison_callback();\
\
	if (callback == NULL) {\
		return memcmp((void *)&key1, (void *)&key2, sizeof(Custom_Key_Type_));\
	}\
	return callback(key1, key2);\
}\
\
/* Assume the first node isn't NULL */\
int Functions_Prefix_##_list_insert(struct Struct_Name_##ListNode *head, Custom_Key_Type_ key,\
			Custom_Value_Type_ value)\
{\
	struct Struct_Name_##ListNode *prev = NULL;\
	size_t iter = 0;\
\
	assert(head != NULL);\
\
	for (; head != NULL; prev = head, head = head->next, iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
\
		if (Functions_Prefix_##_compare_keys(key, head->key) == 0) {\
			/* Override existing value */\
			head->value = value;\
			return 1;\
		}\
	}\
\
	/* Append to end of list */\
	prev->next = Functions_Prefix_##_list_new(NULL, key, value);\
	return 0;\
}\
\
int Functions_Prefix_##_list_find(struct Struct_Name_##ListNode *RESTRICT head, Custom_Key_Type_ key,\
		      Custom_Value_Type_ *RESTRICT out)\
{\
	size_t iter = 0;\
\
	for (iter = 0; head != NULL; head = head->next, iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
\
		if (Functions_Prefix_##_compare_keys(head->key, key) == 0) {\
			if (out != NULL) {\
				*out = head->value;\
			}\
			return 1;\
		}\
	}\
\
	return 0;\
}\
\
int Functions_Prefix_##_list_remove(struct Struct_Name_##ListNode **RESTRICT list, Custom_Key_Type_ key,\
			Custom_Value_Type_ *RESTRICT out)\
{\
	struct Struct_Name_##ListNode *head = NULL;\
	struct Struct_Name_##ListNode *prev = NULL;\
	size_t iter = 0;\
\
	if (list == NULL) {\
		return 0;\
	}\
\
	head = *list;\
\
	for (iter = 0; head != NULL; prev = head, head = head->next, iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
\
		if (Functions_Prefix_##_compare_keys(head->key, key) != 0) {\
			continue;\
		}\
\
		if (out != NULL) {\
			*out = head->value;\
		}\
\
		if (prev == NULL) {\
			*list = head->next;\
		} else {\
			prev->next = head->next;\
		}\
\
		HASHMAP_FREE(head);\
		return 1;\
	}\
\
	return 0;\
}\
\
int Functions_Prefix_##_list_iterate(struct Struct_Name_##ListNode *head,\
			 int (*callback)(Custom_Key_Type_ key, Custom_Value_Type_ value,\
					 void *context),\
			 void *context)\
{\
	size_t iter = 0;\
\
	assert(callback != NULL);\
\
	for (iter = 0; head != NULL; head = head->next, iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
\
		if (callback(head->key, head->value, context) == 0) {\
			return 0;\
		}\
	}\
\
	return 1;\
}\
\
void Functions_Prefix_##_list_free(struct Struct_Name_##ListNode *head)\
{\
	struct Struct_Name_##ListNode *next = NULL;\
	size_t iter = 0;\
\
	for (iter = 0; head != NULL; iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
\
		next = head->next;\
		HASHMAP_FREE(head);\
		head = next;\
	}\
}\
\
struct Struct_Name_##ListNode *Functions_Prefix_##_list_duplicate(struct Struct_Name_##ListNode *head)\
{\
	struct Struct_Name_##ListNode *new_head = NULL;\
	struct Struct_Name_##ListNode *new_next = NULL;\
	size_t iter = 0;\
\
	if (head == NULL) {\
		return NULL;\
	}\
\
	new_head = Functions_Prefix_##_list_new(NULL, head->key, head->value);\
	new_next = new_head;\
\
	for (iter = 0; head->next != NULL;\
	     head = head->next, new_next = new_next->next, iter++) {\
		/* Debug test for infinite loops */\
		assert(iter < 0xFFFFFFFFUL);\
		new_next->next = Functions_Prefix_##_list_new(NULL, head->next->key,\
						  head->next->value);\
	}\
\
	return new_head;\
}\
\
HASHMAP_INLINE unsigned long (*Functions_Prefix_##_compare_hash_callback(void))(Custom_Key_Type_)\
{\
	return Custom_Hash_Func_;\
}\
\
HASHMAP_INLINE size_t Functions_Prefix_##_hash_index(const struct Struct_Name_ *map,\
					 Custom_Key_Type_ key)\
{\
	size_t idx = 0;\
	unsigned long (*callback)(Custom_Key_Type_) = Functions_Prefix_##_compare_hash_callback();\
\
	if (callback == NULL) {\
		idx = Functions_Prefix_##_fnv1a_32_buf((const void *)&key,\
					   sizeof(Custom_Key_Type_));\
	} else {\
		idx = callback(key);\
	}\
\
	idx &= map->capacity - 1;\
\
	return idx;\
}\
\
void Functions_Prefix_##_grow(struct Struct_Name_ *map)\
{\
	size_t new_capacity = 0;\
	struct Struct_Name_ new_map = { 0 };\
	int (*iteration_callback_backup)(Custom_Key_Type_, Custom_Value_Type_, void *);\
	size_t new_capacity_size = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_grow but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	if (map->buckets == NULL) {\
		Functions_Prefix_##_init(map);\
	}\
\
	iteration_callback_backup = map->iteration_callback;\
\
	/* Calculate the next power of 2 */\
	new_capacity = map->capacity;\
	new_capacity |= new_capacity >> 1;\
	new_capacity |= new_capacity >> 2;\
	new_capacity |= new_capacity >> 4;\
	new_capacity |= new_capacity >> 8;\
	if (sizeof(size_t) >= 4) {\
		new_capacity |= new_capacity >> 16;\
	}\
	if (sizeof(size_t) >= 8) {\
		new_capacity |= new_capacity >> 32;\
	}\
	new_capacity++;\
\
	if (new_capacity < map->capacity) {\
		/* Overflow, do not grow, let separate chaining handle\
		 * collisions */\
		return;\
	}\
	if (new_capacity > ((size_t)-1) / sizeof(struct Struct_Name_##ListNode *)) {\
		/* Would overflow, do not grow, let separate chaining handle\
		 * collisions */\
		return;\
	}\
\
	new_capacity_size = new_capacity * sizeof(struct Struct_Name_##ListNode *);\
	new_map.capacity = new_capacity;\
	new_map.buckets = (struct Struct_Name_##ListNode **)HASHMAP_REALLOC(\
		NULL, new_capacity_size);\
	if (new_map.buckets == NULL) {\
		Functions_Prefix_##_panic("Out of memory. Panic.");\
	}\
	memset((void *)new_map.buckets, 0, new_capacity_size);\
\
	map->iteration_callback = Functions_Prefix_##_grow_internal;\
	Functions_Prefix_##_iterate(map, &new_map);\
\
	assert(map->size >= new_map.size);\
	if (map->size != new_map.size) {\
		/* Failed to grow, revert operation */\
		map->iteration_callback = iteration_callback_backup;\
		Functions_Prefix_##_free(&new_map);\
		return;\
	}\
\
	Functions_Prefix_##_free(map);\
	*map = new_map;\
	map->iteration_callback = iteration_callback_backup;\
}\
\
int Functions_Prefix_##_grow_internal(Custom_Key_Type_ key, Custom_Value_Type_ value, void *new_map)\
{\
	/* Should never overwrite */\
	assert(Functions_Prefix_##_insert(new_map, key, value) == 0);\
	return 1;\
}\
\
int Functions_Prefix_##_insert(struct Struct_Name_ *map, Custom_Key_Type_ key, Custom_Value_Type_ value)\
{\
	size_t idx = 0;\
	int overwritten = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return -1;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_insert but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	if (map->buckets == NULL) {\
		Functions_Prefix_##_init(map);\
	}\
\
	/* Check for being above load factor */\
	if ((float)map->buckets_filled / (float)map->capacity >\
	    HASHMAP_LOAD_FACTOR) {\
		Functions_Prefix_##_grow(map);\
	}\
\
	idx = Functions_Prefix_##_hash_index(map, key);\
\
	if (map->buckets[idx] == NULL) {\
		map->buckets[idx] = Functions_Prefix_##_list_new(NULL, key, value);\
		map->buckets_filled++;\
		map->size++;\
		return 0;\
	}\
\
	overwritten = Functions_Prefix_##_list_insert(map->buckets[idx], key, value);\
	if (!overwritten) {\
		map->size++;\
	}\
\
	return overwritten;\
}\
\
int Functions_Prefix_##_remove(struct Struct_Name_ *RESTRICT map, Custom_Key_Type_ key,\
		   Custom_Value_Type_ *RESTRICT out)\
{\
	size_t idx = 0;\
	int found = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return -1;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_remove but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	if (map->buckets == NULL) {\
		return 0;\
	}\
\
	idx = Functions_Prefix_##_hash_index(map, key);\
\
	found = Functions_Prefix_##_list_remove(&map->buckets[idx], key, out);\
\
	if (found) {\
		if (map->buckets[idx] == NULL) {\
			assert(map->buckets_filled > 0);\
			map->buckets_filled--;\
		}\
		assert(map->size > 0);\
		map->size--;\
	}\
\
	return found;\
}\
\
int Functions_Prefix_##_get(const struct Struct_Name_ *RESTRICT map, Custom_Key_Type_ key,\
		Custom_Value_Type_ *RESTRICT out)\
{\
	size_t idx = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return -1;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_get but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	if (map->buckets == NULL) {\
		return 0;\
	}\
\
	idx = Functions_Prefix_##_hash_index(map, key);\
\
	return Functions_Prefix_##_list_find(map->buckets[idx], key, out);\
}\
\
HASHMAP_INLINE int Functions_Prefix_##_has(const struct Struct_Name_ *map, Custom_Key_Type_ key)\
{\
	return Functions_Prefix_##_get(map, key, NULL);\
}\
\
void Functions_Prefix_##_free(struct Struct_Name_ *map)\
{\
	size_t idx = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_free but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	for (idx = 0; idx < map->capacity; idx++) {\
		Functions_Prefix_##_list_free(map->buckets[idx]);\
	}\
\
	HASHMAP_FREE((void *)map->buckets);\
\
	memset((void *)map, 0, sizeof(struct Struct_Name_));\
}\
\
void Functions_Prefix_##_iterate(struct Struct_Name_ *map, void *context)\
{\
	size_t idx = 0;\
	int callback_response = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_iterate but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	if (map->iteration_callback == NULL) {\
		return;\
	}\
\
	for (idx = 0; idx < map->capacity; idx++) {\
		callback_response = Functions_Prefix_##_list_iterate(\
			map->buckets[idx], map->iteration_callback, context);\
		if (callback_response == 0) {\
			break;\
		}\
	}\
}\
\
void Functions_Prefix_##_duplicate(struct Struct_Name_ *dest, struct Struct_Name_ *src)\
{\
	size_t idx = 0;\
\
	if (dest == NULL || src == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_clear but non-null argument expected.");\
	}\
	Functions_Prefix_##_assert(src);\
\
	if (src->capacity == 0) {\
		memset(dest, 0, sizeof(struct Struct_Name_));\
		return;\
	}\
\
	dest->buckets = (struct Struct_Name_##ListNode **)HASHMAP_REALLOC(\
		NULL, src->capacity * sizeof(struct Struct_Name_##ListNode *));\
\
	if (dest->buckets == NULL) {\
		Functions_Prefix_##_panic("Out of memory. Panic.");\
	}\
\
	dest->capacity = src->capacity;\
	dest->size = src->size;\
	dest->buckets_filled = src->buckets_filled;\
	dest->iteration_callback = src->iteration_callback;\
\
	memset((void *)dest->buckets, 0,\
	       dest->capacity * sizeof(struct Struct_Name_##ListNode *));\
\
	for (idx = 0; idx < dest->capacity; idx++) {\
		dest->buckets[idx] = Functions_Prefix_##_list_duplicate(src->buckets[idx]);\
	}\
\
	Functions_Prefix_##_assert(dest);\
}\
\
void Functions_Prefix_##_clear(struct Struct_Name_ *map)\
{\
	size_t idx = 0;\
\
	if (map == NULL) {\
		if (HASHMAP_NO_PANIC_ON_NULL) {\
			return;\
		}\
		Functions_Prefix_##_panic(\
			"Null passed to "#Functions_Prefix_"_clear but non-null argument expected.");\
	}\
\
	Functions_Prefix_##_assert(map);\
\
	for (idx = 0; idx < map->capacity; idx++) {\
		Functions_Prefix_##_list_free(map->buckets[idx]);\
		map->buckets[idx] = NULL;\
	}\
\
	map->size = 0;\
	map->buckets_filled = 0;\
}\
\
unsigned long Functions_Prefix_##_fnv1a_32_buf(const void *buf, size_t len)\
{\
	const unsigned char *bptr = (const unsigned char *)buf;\
	const unsigned char *bend = bptr + len;\
	unsigned long hval = 0x811c9dc5U;\
\
	for (; bptr < bend; bptr++) {\
		hval ^= (unsigned long)bptr[0];\
		hval *= 0x01000193U;\
		hval &= 0xFFFFFFFFUL;\
	}\
\
	return hval;\
}\
\
unsigned long Functions_Prefix_##_fnv1a_32_str(const char *str)\
{\
	const unsigned char *ustr = (const unsigned char *)str;\
	unsigned long hval = 0x811c9dc5U;\
\
	for (; ustr[0] != '\0'; ustr++) {\
		hval ^= (unsigned long)ustr[0];\
		hval *= 0x01000193U;\
		hval &= 0xFFFFFFFFUL;\
	}\
\
	return hval;\
}

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
