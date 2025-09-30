#include "unity/unity.h"
#include "hashmap_generated.h"

jmp_buf abort_jmp;

void setUp(void)
{
}

void tearDown(void)
{
}

void test_init_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_init(NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_grow_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_grow(NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_insert_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_insert(NULL, "hello", 10);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_remove_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_remove(NULL, "hello", NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_get_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_get(NULL, "hello", NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_has_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_has(NULL, "hello");
	} else {
		return;
	}
	TEST_FAIL();
}

void test_free_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_free(NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_iterate_pass_null_abort(void)
{
	if (setjmp(abort_jmp) == 0) {
		hashmap_iterate(NULL, NULL);
	} else {
		return;
	}
	TEST_FAIL();
}

void test_duplicate_pass_null_abort_src(void)
{
	/* TODO: implement duplicating and this test */
	/* Hashmap src;
	 * if (setjmp(abort_jmp) == 0) {
	 * 	hashmap_duplicate(NULL, &src);
	 * } else {
	 * 	return;
	 * }
	 * TEST_FAIL(); */
}

void test_duplicate_pass_null_abort_dest(void)
{
	/* TODO: implement duplicating and this test */
	/* Hashmap dest;
	 * if (setjmp(abort_jmp) == 0) {
	 * 	hashmap_duplicate(&dest, NULL);
	 * } else {
	 * 	return;
	 * }
	 * TEST_FAIL(); */
}

void test_duplicate_pass_null_abort_both(void)
{
	/* TODO: implement duplicating and this test */
	/* if (setjmp(abort_jmp) == 0) {
	 * 	hashmap_duplicate(NULL, NULL);
	 * } else {
	 * 	return;
	 * }
	 * TEST_FAIL(); */
}

void test_clear_pass_null_abort(void)
{
	/* TODO: implement clearing and this test */
	/* if (setjmp(abort_jmp) == 0) {
	 * 	hashmap_clear(NULL);
	 * } else {
	 * 	return;
	 * }
	 * TEST_FAIL(); */
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_init_pass_null_abort);
	RUN_TEST(test_grow_pass_null_abort);
	RUN_TEST(test_insert_pass_null_abort);
	RUN_TEST(test_remove_pass_null_abort);
	RUN_TEST(test_get_pass_null_abort);
	RUN_TEST(test_has_pass_null_abort);
	RUN_TEST(test_free_pass_null_abort);
	RUN_TEST(test_iterate_pass_null_abort);
	RUN_TEST(test_duplicate_pass_null_abort_src);
	RUN_TEST(test_duplicate_pass_null_abort_dest);
	RUN_TEST(test_duplicate_pass_null_abort_both);
	RUN_TEST(test_clear_pass_null_abort);

	return UNITY_END();
}
