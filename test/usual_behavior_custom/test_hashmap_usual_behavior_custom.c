#include <assert.h>
#include <string.h>

#include "unity/unity.h"
#include "hashmap_generated.h"

enum { TEST_STRESS_MULTIPLIER = 1000U, TEST_ITERATIONS_BREAK = 50U };

jmp_buf abort_jmp;

const char *test_strings[] = {
	"hello",    "world",	"dragons!", "testing", "hashmap",    "function",
	"alice",    "bob",	"charlie",  "delta",   "echo",	     "foxtrot",
	"apple",    "banana",	"cherry",   "date",    "elderberry", "fig",
	"red",	    "green",	"blue",	    "yellow",  "purple",     "orange",
	"cat",	    "dog",	"bird",	    "fish",    "rabbit",     "hamster",
	"mountain", "river",	"ocean",    "forest",  "desert",     "valley",
	"quick",    "brown",	"fox",	    "jumps",   "over",	     "lazy",
	"alpha",    "beta",	"gamma",    "theta",   "omega",	     "sigma",
	"one",	    "two",	"three",    "four",    "five",	     "six",
	"january",  "february", "march",    "april",   "may",	     "june",
	"coffee",   "tea",	"water",    "juice",   "milk",	     "soda",
	"keyboard", "mouse",	"screen",   "laptop",  "desktop",    "tablet",
	"happy",    "sad",	"angry",    "excited", "calm",	     "tired",
	"north",    "south",	"east",	    "west",    "center",     "edge",
	"start",    "middle",	"end",	    "begin",   "finish",     "complete",
	"tiny",	    "small",	"medium",   "large",   "huge",	     "giant",
	"fast",	    "slow",	"gauss",    "rapid",   "swift",	     "gradual",
	"light",    "dark",	"bright",   "dim",     "shadow",     "glow"
};
const size_t test_strings_size = sizeof(test_strings) / sizeof(const char *);

Hashmap get_garbage_map(void)
{
	Hashmap map = { 0 };

	map.buckets = (struct HashmapListNode **)0xDEADBEEF;
	map.iteration_callback = (int (*)(const char *, int, void *))0xDEADBEEF;
	map.size = 0xDEADBEEF;
	map.buckets_filled = 0xDEADBEEF;
	map.capacity = 0xDEADBEEF;

	return map;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_custom_comparison(void)
{
	TEST_ASSERT_EQUAL(strcmp, hashmap_compare_comparison_callback());
}

void test_custom_hash(void)
{
	TEST_ASSERT_EQUAL(fnv1a_32_str, hashmap_compare_hash_callback());
}

void test_init_from_zero(void)
{
	Hashmap map = { 0 };
	hashmap_init(&map);

	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	hashmap_free(&map);
}

void test_init_from_garbage(void)
{
	Hashmap map = get_garbage_map();

	hashmap_init(&map);

	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	hashmap_free(&map);
}

void test_grow_from_zero(void)
{
	Hashmap map = { 0 };
	hashmap_grow(&map);
	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY << 1, map.capacity);
	hashmap_free(&map);
}

void test_grow_overflow1(void)
{
	Hashmap map = { 0 };
	struct HashmapListNode **old_buckets;
	size_t expected_capacity = (size_t)-1;

	hashmap_init(&map);
	old_buckets = map.buckets;

	/* Find previous power of 2 from SIZE_MAX to cause overflow */
	expected_capacity |= expected_capacity >> 1;
	expected_capacity |= expected_capacity >> 2;
	expected_capacity |= expected_capacity >> 4;
	expected_capacity |= expected_capacity >> 8;
	if (sizeof(size_t) >= 4) {
		expected_capacity |= expected_capacity >> 16;
	}
	if (sizeof(size_t) >= 8) {
		expected_capacity |= expected_capacity >> 32;
	}
	expected_capacity = (expected_capacity >> 1) + 1;

	map.capacity = expected_capacity;

	/* Should be a no-op */
	hashmap_grow(&map);

	TEST_ASSERT_EQUAL(old_buckets, map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(expected_capacity, map.capacity);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);

	/* Back to normal capacity */
	map.capacity = HASHMAP_DEFAULT_CAPACITY;
	hashmap_free(&map);
}

void test_grow_overflow2(void)
{
	Hashmap map = { 0 };
	struct HashmapListNode **old_buckets;
	size_t expected_capacity =
		(size_t)-1 / sizeof(struct HashmapListNode *);

	hashmap_init(&map);
	old_buckets = map.buckets;

	/* Find next power of 2 from the minimum value needed to cause
	 * overflow */
	expected_capacity |= expected_capacity >> 1;
	expected_capacity |= expected_capacity >> 2;
	expected_capacity |= expected_capacity >> 4;
	expected_capacity |= expected_capacity >> 8;
	if (sizeof(size_t) >= 4) {
		expected_capacity |= expected_capacity >> 16;
	}
	if (sizeof(size_t) >= 8) {
		expected_capacity |= expected_capacity >> 32;
	}
	expected_capacity++;

	map.capacity = expected_capacity;

	/* Should be a no-op */
	hashmap_grow(&map);

	TEST_ASSERT_EQUAL(old_buckets, map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(expected_capacity, map.capacity);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);

	/* Back to normal capacity */
	map.capacity = HASHMAP_DEFAULT_CAPACITY;
	hashmap_free(&map);
}

void test_grow(void)
{
	Hashmap map = { 0 };
	Hashmap previous_map = { 0 };
	size_t idx = 0;
	float previous_load = 0;

	for (idx = 0; idx < test_strings_size; idx++) {
		previous_load = (float)map.buckets_filled / (float)map.capacity;
		previous_map = map;
		hashmap_insert(&map, test_strings[idx], (int)idx);
		if (previous_load >
		    (float)map.buckets_filled / (float)map.capacity) {
			TEST_ASSERT_EQUAL_UINT(previous_map.capacity << 1,
					       map.capacity);
			TEST_ASSERT_EQUAL_UINT(previous_map.size + 1, map.size);
			TEST_ASSERT_NOT_EQUAL(previous_map.buckets,
					      map.buckets);
			TEST_ASSERT_EQUAL(previous_map.iteration_callback,
					  map.iteration_callback);
		}
	}

	hashmap_free(&map);
}

void test_insert_from_zero(void)
{
	Hashmap map = { 0 };
	int out = 0;

	hashmap_insert(&map, "hello", 10);

	TEST_ASSERT_EQUAL_UINT(1, map.size);
	TEST_ASSERT_EQUAL_UINT(1, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);
	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);

	TEST_ASSERT_EQUAL_INT(1, hashmap_get(&map, "hello", &out));
	TEST_ASSERT_EQUAL_INT(10, out);

	hashmap_free(&map);
}

void test_insert_and_find(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int replaced = 0;
	int found = 0;
	int gotten = 0;
	const size_t max_idx = test_strings_size * TEST_STRESS_MULTIPLIER;

	hashmap_init(&map);

	for (idx = 0, gotten = -1; idx < max_idx; idx++) {
		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    &gotten);
		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_INT(-1, gotten);
		TEST_ASSERT_EQUAL_UINT(0, map.size);
		TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
		TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);
	}

	for (idx = 0; idx < max_idx; idx++) {
		gotten = -1;
		if (idx < test_strings_size) {
			found = hashmap_get(
				&map, test_strings[idx % test_strings_size],
				&gotten);
			TEST_ASSERT_EQUAL_INT(0, found);
			TEST_ASSERT_EQUAL_INT(-1, gotten);
		}

		replaced = hashmap_insert(
			&map, test_strings[idx % test_strings_size], (int)idx);

		if (idx < test_strings_size) {
			TEST_ASSERT_EQUAL_UINT(idx + 1, map.size);
		} else {
			TEST_ASSERT_EQUAL_UINT(test_strings_size, map.size);
		}
		TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);
		TEST_ASSERT_GREATER_THAN_UINT(0, map.buckets_filled);

		if (idx < test_strings_size) {
			TEST_ASSERT_EQUAL_INT(0, replaced);
		} else {
			TEST_ASSERT_EQUAL_INT(1, replaced);
		}

		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    &gotten);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(idx, gotten);
	}

	for (idx = 0; idx < max_idx; idx++) {
		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    &gotten);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(max_idx - test_strings_size +
					      (idx % test_strings_size),
				      gotten);
	}

	hashmap_free(&map);
}

void test_insert_and_find_no_get_out(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int replaced = 0;
	int found = 0;
	const size_t max_idx = test_strings_size * TEST_STRESS_MULTIPLIER;

	hashmap_init(&map);

	for (idx = 0; idx < max_idx; idx++) {
		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    NULL);
		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_UINT(0, map.size);
		TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
		TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);
	}

	for (idx = 0; idx < max_idx; idx++) {
		if (idx < test_strings_size) {
			found = hashmap_get(
				&map, test_strings[idx % test_strings_size],
				NULL);
			TEST_ASSERT_EQUAL_INT(0, found);
		}

		replaced = hashmap_insert(
			&map, test_strings[idx % test_strings_size], (int)idx);

		if (idx < test_strings_size) {
			TEST_ASSERT_EQUAL_UINT(idx + 1, map.size);
		} else {
			TEST_ASSERT_EQUAL_UINT(test_strings_size, map.size);
		}
		TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);
		TEST_ASSERT_GREATER_THAN_UINT(0, map.buckets_filled);

		if (idx < test_strings_size) {
			TEST_ASSERT_EQUAL_INT(0, replaced);
		} else {
			TEST_ASSERT_EQUAL_INT(1, replaced);
		}

		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    NULL);
		TEST_ASSERT_EQUAL_INT(1, found);
	}

	for (idx = 0; idx < max_idx; idx++) {
		found = hashmap_get(&map, test_strings[idx % test_strings_size],
				    NULL);
		TEST_ASSERT_EQUAL_INT(1, found);
	}

	hashmap_free(&map);
}

void test_insert_no_init(void)
{
	Hashmap map = { 0 };

	hashmap_insert(&map, "hello", 10);

	TEST_ASSERT_EQUAL_UINT(1, map.size);
	TEST_ASSERT_EQUAL_UINT(1, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);
	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);

	hashmap_free(&map);
}

void test_remove_from_zero(void)
{
	Hashmap map = { 0 };
	Hashmap expected = { 0 };
	int out = 0;

	TEST_ASSERT_EQUAL_MEMORY(&expected, &map, sizeof(Hashmap));

	TEST_ASSERT_EQUAL_INT(0, hashmap_remove(&map, "hello", &out));
	TEST_ASSERT_EQUAL_INT(0, out);
}

void test_remove_half_even(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int removed = 0;
	int gotten = 0;
	int found = 0;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	for (idx = 0; idx < test_strings_size; idx += 2) {
		found = hashmap_remove(&map, test_strings[idx], &removed);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(idx, removed);
	}

	TEST_ASSERT_EQUAL_UINT(test_strings_size / 2, map.size);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.buckets_filled);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_get(&map, test_strings[idx], &gotten);

		if (idx % 2 == 0) {
			TEST_ASSERT_EQUAL_INT(0, found);
			TEST_ASSERT_EQUAL_INT(idx == 0 ? 0 : idx - 1, gotten);
		} else {
			TEST_ASSERT_EQUAL_INT(1, found);
			TEST_ASSERT_EQUAL_INT(idx, gotten);
		}
	}

	hashmap_free(&map);
}

void test_remove_half_odd(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int removed = 0;
	int gotten = 0;
	int found = 0;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	for (idx = 1; idx < test_strings_size; idx += 2) {
		found = hashmap_remove(&map, test_strings[idx], &removed);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(idx, removed);
	}

	TEST_ASSERT_EQUAL_UINT(test_strings_size / 2, map.size);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.buckets_filled);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_get(&map, test_strings[idx], &gotten);

		if (idx % 2 == 0) {
			TEST_ASSERT_EQUAL_INT(1, found);
			TEST_ASSERT_EQUAL_INT(idx, gotten);
		} else {
			TEST_ASSERT_EQUAL_INT(0, found);
			TEST_ASSERT_EQUAL_INT(idx - 1, gotten);
		}
	}

	hashmap_free(&map);
}

void test_remove_all(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int removed = 0;
	int gotten = 0;
	int found = 0;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_remove(&map, test_strings[idx], &removed);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(idx, removed);
	}

	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_get(&map, test_strings[idx], &gotten);

		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_INT(0, gotten);
	}

	hashmap_free(&map);
}

void test_remove_no_out(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int gotten = 0;
	int found = 0;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_remove(&map, test_strings[idx], NULL);
		TEST_ASSERT_EQUAL_INT(1, found);
	}

	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_GREATER_THAN_UINT(0, map.capacity);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_get(&map, test_strings[idx], &gotten);

		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_INT(0, gotten);
	}

	hashmap_free(&map);
}

void test_remove_not_inserted(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	int removed = 0;
	int gotten = 0;
	int found = 0;

	hashmap_init(&map);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_remove(&map, test_strings[idx], &removed);
		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_INT(0, removed);
	}

	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	for (idx = 0; idx < test_strings_size; idx++) {
		found = hashmap_get(&map, test_strings[idx], &gotten);

		TEST_ASSERT_EQUAL_INT(0, found);
		TEST_ASSERT_EQUAL_INT(0, gotten);
	}

	hashmap_free(&map);
}

void test_insert_and_remove(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	size_t inserting = 0;
	int replaced = 0;
	int added_total = 0;
	int removed_total = 0;
	int removed_value = 0;

	const char *test_string = NULL;
	const size_t max_idx = test_strings_size * TEST_STRESS_MULTIPLIER;

	hashmap_init(&map);

	for (idx = 0; idx < max_idx; idx++) {
		test_string = test_strings[idx % test_strings_size];

		/* Randomly choose whether we insert or delete */
		inserting = idx;
		inserting ^= inserting >> 16;
		inserting *= 0x85ebca6b;
		inserting ^= inserting >> 13;
		inserting *= 0xc2b2ae35;
		inserting ^= inserting >> 16;

		if (inserting % 2 == 0) {
			/* Inserting */
			replaced = hashmap_insert(&map, test_string, (int)idx);
			if (idx < test_strings_size) {
				TEST_ASSERT_EQUAL_INT(0, replaced);
			}
			if (replaced == 0) {
				added_total++;
			}
		} else {
			/* Deleting */
			removed_total += hashmap_remove(&map, test_string,
							&removed_value);
		}

		TEST_ASSERT_EQUAL_UINT(added_total - removed_total, map.size);
	}

	hashmap_free(&map);
}

void test_free(void)
{
	Hashmap map = { 0 };
	Hashmap map_zero = { 0 };
	hashmap_init(&map);
	hashmap_free(&map);
	TEST_ASSERT_EQUAL_MEMORY(&map_zero, &map, sizeof(Hashmap));
}

void test_free_zero(void)
{
	Hashmap map = { 0 };
	Hashmap map_zero = { 0 };
	hashmap_free(&map);
	TEST_ASSERT_EQUAL_MEMORY(&map_zero, &map, sizeof(Hashmap));
}

int test_iterate_with_context_callback(const char *key, int val, void *context)
{
	const char **test_strings_context = NULL;
	size_t idx = 0;

	TEST_ASSERT_NOT_NULL(key);
	TEST_ASSERT_NOT_NULL(context);
	TEST_ASSERT_GREATER_OR_EQUAL_INT(0, val);

	test_strings_context = (const char **)context;

	for (idx = 0; idx < test_strings_size; idx++) {
		if (strcmp(test_strings_context[idx], key) == 0) {
			TEST_ASSERT_EQUAL_INT(idx, val);
			break;
		}
	}

	return 1;
}

int test_iterate_no_context_callback(const char *key, int val, void *context)
{
	size_t idx = 0;

	TEST_ASSERT_NOT_NULL(key);
	TEST_ASSERT_NULL(context);
	TEST_ASSERT_GREATER_OR_EQUAL_INT(0, val);

	for (idx = 0; idx < test_strings_size; idx++) {
		if (strcmp(test_strings[idx], key) == 0) {
			TEST_ASSERT_EQUAL_INT(idx, val);
			break;
		}
	}

	return 1;
}

int test_iterate_break_after_x_iter(const char *key, int val, void *context)
{
	size_t idx = 0;
	size_t *total_iteration_count = (size_t *)context;

	TEST_ASSERT_NOT_NULL(key);
	TEST_ASSERT_NOT_NULL(context);
	TEST_ASSERT_GREATER_OR_EQUAL_INT(0, val);

	if (*total_iteration_count >= TEST_ITERATIONS_BREAK) {
		return 0;
	}

	for (idx = 0; idx < test_strings_size; idx++) {
		if (strcmp(test_strings[idx], key) == 0) {
			TEST_ASSERT_EQUAL_INT(idx, val);
			break;
		}
	}

	*total_iteration_count += 1;

	return 1;
}

int test_iterate_empty_function(const char *key, int val, void *context)
{
	(void)key;
	(void)val;
	(void)context;
	return 1;
}

void test_iterate_with_context(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;

	hashmap_init(&map);

	map.iteration_callback = test_iterate_with_context_callback;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	hashmap_iterate(&map, (void *)test_strings);

	hashmap_free(&map);
}

void test_iterate_no_context(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;

	hashmap_init(&map);

	map.iteration_callback = test_iterate_no_context_callback;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	hashmap_iterate(&map, NULL);

	hashmap_free(&map);
}

void test_iterate_no_callback(void)
{
	Hashmap map = { 0 };
	hashmap_init(&map);
	hashmap_iterate(&map, NULL);
	hashmap_free(&map);
}

void test_iterate_break(void)
{
	Hashmap map = { 0 };
	size_t idx = 0;
	size_t iterations = 0;

	hashmap_init(&map);

	map.iteration_callback = test_iterate_break_after_x_iter;

	for (idx = 0; idx < test_strings_size; idx++) {
		hashmap_insert(&map, test_strings[idx], (int)idx);
	}

	hashmap_iterate(&map, &iterations);

	TEST_ASSERT_EQUAL_UINT(TEST_ITERATIONS_BREAK, iterations);

	hashmap_free(&map);
}

void test_iterate_from_almost_zero(void)
{
	Hashmap map = { 0 };
	Hashmap expected = { 0 };

	map.iteration_callback = test_iterate_empty_function;
	expected.iteration_callback = test_iterate_empty_function;

	hashmap_iterate(&map, NULL);

	TEST_ASSERT_EQUAL_MEMORY(&map, &expected, sizeof(Hashmap));
}

void test_iterate_from_zero(void)
{
	Hashmap map = { 0 };
	Hashmap expected = { 0 };

	hashmap_iterate(&map, NULL);

	TEST_ASSERT_EQUAL_MEMORY(&map, &expected, sizeof(Hashmap));
}

void test_duplicate_from_zero(void)
{
	Hashmap expected = { 0 };
	Hashmap src = { 0 };
	Hashmap dest = get_garbage_map(); /* To be overwritten */

	hashmap_duplicate(&dest, &src);

	TEST_ASSERT_EQUAL_MEMORY(&expected, &src, sizeof(Hashmap));
	TEST_ASSERT_EQUAL_MEMORY(&expected, &dest, sizeof(Hashmap));
}

void test_duplicate_to_zero(void)
{
	Hashmap src = { 0 };
	Hashmap dest = { 0 };
	int gotten = 0;

	hashmap_insert(&src, "hello", 10);
	hashmap_duplicate(&dest, &src);

	TEST_ASSERT_EQUAL_UINT(src.size, dest.size);
	TEST_ASSERT_EQUAL_UINT(src.capacity, dest.capacity);
	TEST_ASSERT_EQUAL_UINT(src.buckets_filled, dest.buckets_filled);
	TEST_ASSERT_EQUAL(src.iteration_callback, dest.iteration_callback);
	TEST_ASSERT_NOT_EQUAL(src.buckets, dest.buckets);

	TEST_ASSERT_EQUAL_INT(1, hashmap_get(&dest, "hello", &gotten));
	TEST_ASSERT_EQUAL_INT(10, gotten);

	hashmap_free(&src);
	hashmap_free(&dest);
}

void test_clear_zero(void)
{
	Hashmap map = { 0 };
	hashmap_clear(&map);
	TEST_ASSERT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.capacity);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
}

void test_clear(void)
{
	Hashmap map = { 0 };

	hashmap_insert(&map, "hello", 10);
	hashmap_clear(&map);

	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_UINT(0, map.size);
	TEST_ASSERT_EQUAL_UINT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_UINT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	hashmap_free(&map);
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_custom_comparison);
	RUN_TEST(test_custom_hash);
	RUN_TEST(test_init_from_zero);
	RUN_TEST(test_grow_from_zero);
	RUN_TEST(test_grow_overflow1);
	RUN_TEST(test_grow_overflow2);
	RUN_TEST(test_grow);
	RUN_TEST(test_insert_from_zero);
	RUN_TEST(test_insert_and_find);
	RUN_TEST(test_insert_and_find_no_get_out);
	RUN_TEST(test_insert_no_init);
	RUN_TEST(test_insert_and_remove);
	RUN_TEST(test_remove_from_zero);
	RUN_TEST(test_remove_half_even);
	RUN_TEST(test_remove_half_odd);
	RUN_TEST(test_remove_all);
	RUN_TEST(test_remove_no_out);
	RUN_TEST(test_remove_not_inserted);
	RUN_TEST(test_free);
	RUN_TEST(test_free_zero);
	RUN_TEST(test_iterate_with_context);
	RUN_TEST(test_iterate_no_context);
	RUN_TEST(test_iterate_no_callback);
	RUN_TEST(test_iterate_break);
	RUN_TEST(test_iterate_from_zero);
	RUN_TEST(test_iterate_from_almost_zero);
	RUN_TEST(test_duplicate_from_zero);
	RUN_TEST(test_duplicate_to_zero);
	RUN_TEST(test_clear_zero);
	RUN_TEST(test_clear);

	return UNITY_END();
}
