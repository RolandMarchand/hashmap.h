#include "unity/unity.h"
#include "hashmap_generated.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_init_pass_null_ignore(void)
{
	hashmap_init(NULL);
}

void test_grow_pass_null_ignore(void)
{
	hashmap_grow(NULL);
}

void test_insert_pass_null_ignore(void)
{
	hashmap_insert(NULL, "hello", 10);
}

void test_remove_pass_null_ignore(void)
{
	hashmap_remove(NULL, "hello", NULL);
}

void test_get_pass_null_ignore(void)
{
	hashmap_get(NULL, "hello", NULL);
}

void test_has_pass_null_ignore(void)
{
	hashmap_has(NULL, "hello");
}

void test_free_pass_null_ignore(void)
{
	hashmap_free(NULL);
}

void test_iterate_pass_null_ignore(void)
{
	hashmap_iterate(NULL, NULL);
}

void test_duplicate_pass_null_ignore_src(void)
{
	/* TODO: implement duplicating and this test */
	/* Hashmap src;
	 * hashmap_duplicate(NULL, &src); */
}

void test_duplicate_pass_null_ignore_dest(void)
{
	/* TODO: implement duplicating and this test */
	/* Hashmap dest;
	 * hashmap_duplicate(&dest, NULL); */
}

void test_duplicate_pass_null_ignore_both(void)
{
	/* TODO: implement duplicating and this test */
	/* hashmap_duplicate(NULL, NULL); */
}

void test_clear_pass_null_ignore(void)
{
	/* TODO: implement clearing and this test */
	/* hashmap_clear(NULL); */
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_init_pass_null_ignore);
	RUN_TEST(test_grow_pass_null_ignore);
	RUN_TEST(test_insert_pass_null_ignore);
	RUN_TEST(test_remove_pass_null_ignore);
	RUN_TEST(test_get_pass_null_ignore);
	RUN_TEST(test_has_pass_null_ignore);
	RUN_TEST(test_free_pass_null_ignore);
	RUN_TEST(test_iterate_pass_null_ignore);
	RUN_TEST(test_duplicate_pass_null_ignore_src);
	RUN_TEST(test_duplicate_pass_null_ignore_dest);
	RUN_TEST(test_duplicate_pass_null_ignore_both);
	RUN_TEST(test_clear_pass_null_ignore);

	return UNITY_END();
}
