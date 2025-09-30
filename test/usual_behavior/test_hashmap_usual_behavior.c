#include "hashmap.h"
#include "unity/unity.h"
#include "hashmap_generated.h"

jmp_buf abort_jmp;

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

void test_init_from_zero(void)
{
	Hashmap map = { 0 };
	hashmap_init(&map);

	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_INT(0, map.size);
	TEST_ASSERT_EQUAL_INT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_INT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	hashmap_free(&map);
}

void test_init_from_garbage(void)
{
	Hashmap map = get_garbage_map();

	hashmap_init(&map);

	TEST_ASSERT_NOT_NULL(map.buckets);
	TEST_ASSERT_NULL(map.iteration_callback);
	TEST_ASSERT_EQUAL_INT(0, map.size);
	TEST_ASSERT_EQUAL_INT(0, map.buckets_filled);
	TEST_ASSERT_EQUAL_INT(HASHMAP_DEFAULT_CAPACITY, map.capacity);

	hashmap_free(&map);
}

void test_grow_from_zero(void)
{
	/* TODO: implement growing and write this test */
}

void test_grow_from_garbage(void)
{
	/* TODO: implement growing and write this test */
}

void test_grow_overflow(void)
{
	/* TODO: implement growing and write this test */
}

void test_grow_many(void)
{
	/* TODO: implement growing and write this test */
}

void test_insert_and_find(void)
{
	Hashmap map = { 0 };
	int idx = 0;
	int replaced = 0;
	int found = 0;
	int gotten = 0;
	const char *test_strings[] = {
		"hello",    "world",	  "dragons!", "testing", "hashmap",
		"function", "alice",	  "bob",      "charlie", "delta",
		"echo",	    "foxtrot",	  "apple",    "banana",	 "cherry",
		"date",	    "elderberry", "fig",      "red",	 "green",
		"blue",	    "yellow",	  "purple",   "orange",	 "cat",
		"dog",	    "bird",	  "fish",     "rabbit",	 "hamster",
		"mountain", "river",	  "ocean",    "forest",	 "desert",
		"valley",   "quick",	  "brown",    "fox",	 "jumps",
		"over",	    "lazy",	  "alpha",    "beta",	 "gamma",
		"theta",    "omega",	  "sigma",    "one",	 "two",
		"three",    "four",	  "five",     "six",	 "january",
		"february", "march",	  "april",    "may",	 "june",
		"coffee",   "tea",	  "water",    "juice",	 "milk",
		"soda",	    "keyboard",	  "mouse",    "screen",	 "laptop",
		"desktop",  "tablet",	  "happy",    "sad",	 "angry",
		"excited",  "calm",	  "tired",    "north",	 "south",
		"east",	    "west",	  "center",   "edge",	 "start",
		"middle",   "end",	  "begin",    "finish",	 "complete",
		"tiny",	    "small",	  "medium",   "large",	 "huge",
		"giant",    "fast",	  "slow",     "gauss",	 "rapid",
		"swift",    "gradual",	  "light",    "dark",	 "bright",
		"dim",	    "shadow",	  "glow"
	};
	int test_strings_size = sizeof(test_strings) / sizeof(const char *);
	const int max_idx = test_strings_size * 100;

	hashmap_init(&map);

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
			&map, test_strings[idx % test_strings_size], idx);

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
		printf("%d\n", idx);
		TEST_ASSERT_EQUAL_INT(1, found);
		TEST_ASSERT_EQUAL_INT(max_idx - test_strings_size +
					      (idx % test_strings_size),
				      gotten);
	}

	hashmap_free(&map);
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_init_from_zero);
	RUN_TEST(test_init_from_garbage);
	RUN_TEST(test_grow_from_zero);
	RUN_TEST(test_grow_from_garbage);
	RUN_TEST(test_grow_overflow);
	RUN_TEST(test_grow_many);
	RUN_TEST(test_insert_and_find);

	return UNITY_END();
}
