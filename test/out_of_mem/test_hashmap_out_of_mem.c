#include "unity/unity.h"
#include "hashmap_generated.h"

jmp_buf abort_jmp;

int disabled_realloc = 0;

void setUp(void)
{
	disabled_realloc = 0;
}

void tearDown(void)
{
}

void *return_null(void *ptr, size_t size)
{
	if (disabled_realloc) {
		return NULL;
	}
	return realloc(ptr, size);
}

void test_grow_out_of_mem(void)
{
	/* TODO: add implement hashmap_grow and this test */
	/* Hashmap map = { 0 };
	 * 
	 * /\* Bypass would-be-failing initialization *\/
	 * map.capacity = HASHMAP_DEFAULT_CAPACITY;
	 * map.buckets = (struct HashmapListNode **)realloc(
	 * 	NULL, map.capacity * sizeof(struct HashmapListNode *));
	 * memset((void *)map.buckets, 0,
	 *        map.capacity * sizeof(struct HashmapListNode *));
	 * 
	 * if (setjmp(abort_jmp) == 0) {
	 * 	hashmap_grow(&map);
	 * } else {
	 * 	hashmap_free(&map);
	 * 	return;
	 * }
	 * 
	 * hashmap_free(&map);
	 * TEST_FAIL(); */
}

void test_init_out_of_mem(void)
{
	Hashmap map = { 0 };

	disabled_realloc = 1;

	if (setjmp(abort_jmp) == 0) {
		hashmap_init(&map);
	} else {
		hashmap_free(&map);
		return;
	}

	hashmap_free(&map);
	TEST_FAIL();
}

void test_insert_out_of_mem(void)
{
	Hashmap map = { 0 };

	hashmap_init(&map);
	disabled_realloc = 1;

	if (setjmp(abort_jmp) == 0) {
		hashmap_insert(&map, "hello", 10);
	} else {
		hashmap_free(&map);
		return;
	}

	hashmap_free(&map);
	TEST_FAIL();
}

void test_duplicate_out_of_mem(void)
{
	Hashmap src = { 0 };
	Hashmap dest = { 0 };

	hashmap_insert(&src, "hi", 10);
	disabled_realloc = 1;

	if (setjmp(abort_jmp) == 0) {
		hashmap_duplicate(&dest, &src);
	} else {
		hashmap_free(&src);
		return;
	}

	hashmap_free(&src);
	hashmap_free(&dest);
	TEST_FAIL();
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_grow_out_of_mem);
	RUN_TEST(test_init_out_of_mem);
	RUN_TEST(test_insert_out_of_mem);
	RUN_TEST(test_duplicate_out_of_mem);

	return UNITY_END();
}
