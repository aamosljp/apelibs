#define APEDSA_HASHMAP_DOUBLE_HASHING
#include "test.h"
#include "apedsa_api.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

TEST(da_push_one)
{
	int *arr = NULL;
	apedsa_da_push(arr, 1);
	ASSERT_EQ(apedsa_da_count(arr), 1);
	ASSERT_EQ(arr[0], 1);
	return PASSED;
}

TEST(da_push_many)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	ASSERT_GE(apedsa_da_cap(arr), 100);
	for (int i = 0; i < 100; i++) {
		ASSERT_EQ(arr[i], i);
	}
	return PASSED;
}

TEST(da_insert_first)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_insert(arr, 0, 200);
	ASSERT_EQ(apedsa_da_count(arr), 101);
	ASSERT_EQ(arr[0], 200);
	for (int i = 1; i < 101; i++) {
		ASSERT_EQ(arr[i], i - 1);
	}
	return PASSED;
}

TEST(da_insert_last)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_insert(arr, 100, 200);
	ASSERT_EQ(apedsa_da_count(arr), 101);
	ASSERT_EQ(arr[100], 200);
	for (int i = 0; i < 100; i++) {
		ASSERT_EQ(arr[i], i);
	}
	return PASSED;
}

TEST(da_insert_middle)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_insert(arr, 50, 200);
	ASSERT_EQ(apedsa_da_count(arr), 101);
	ASSERT_EQ(arr[50], 200);
	for (int i = 0; i < 50; i++) {
		ASSERT_EQ(arr[i], i);
	}
	for (int i = 51; i < 101; i++) {
		ASSERT_EQ(arr[i], i - 1);
	}
	return PASSED;
}

TEST(da_delete_first)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_delete(arr, 0);
	ASSERT_EQ(apedsa_da_count(arr), 99);
	for (int i = 0; i < 99; i++) {
		ASSERT_EQ(arr[i], i + 1);
	}
	return PASSED;
}

TEST(da_delete_last)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_delete(arr, 99);
	ASSERT_EQ(apedsa_da_count(arr), 99);
	for (int i = 0; i < 99; i++) {
		ASSERT_EQ(arr[i], i);
	}
	return PASSED;
}

TEST(da_delete_middle)
{
	int *arr = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_da_push(arr, i);
	}
	ASSERT_EQ(apedsa_da_count(arr), 100);
	apedsa_da_delete(arr, 50);
	ASSERT_EQ(apedsa_da_count(arr), 99);
	for (int i = 0; i < 49; i++) {
		ASSERT_EQ(arr[i], i);
	}
	for (int i = 50; i < 99; i++) {
		ASSERT_EQ(arr[i], i + 1);
	}
	return PASSED;
}

static void run_da_tests(void)
{
	LOG_INFO("DA tests:");
	RUN_TEST(da_push_one);
	RUN_TEST(da_push_many);
	RUN_TEST(da_insert_first);
	RUN_TEST(da_insert_last);
	RUN_TEST(da_insert_middle);
	RUN_TEST(da_delete_first);
	RUN_TEST(da_delete_last);
	RUN_TEST(da_delete_middle);
}

typedef struct {
	char *key;
	int value;
} Kv;

typedef struct {
	int key;
	int value;
} Ki;

TEST(hm_string_put_one)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "hello", 5);
	ASSERT_EQ(apedsa_shm_get(map, "hello"), 5);
	return PASSED;
}

TEST(hm_string_put_many)
{
	Kv *map = NULL;
	for (int i = 0; i < 50; i++) {
		char key[20];
		sprintf(key, "key-%d", i);
		apedsa_shm_put(map, key, i * 10);
	}
	for (int i = 0; i < 50; i++) {
		char key[20];
		sprintf(key, "key-%d", i);
		ASSERT_EQ(apedsa_shm_get(map, key), i * 10);
	}
	return PASSED;
}

TEST(hm_string_update_existing)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "test", 100);
	ASSERT_EQ(apedsa_shm_get(map, "test"), 100);
	apedsa_shm_put(map, "test", 200);
	ASSERT_EQ(apedsa_shm_get(map, "test"), 200);
	return PASSED;
}

TEST(hm_string_delete)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "a", 1);
	apedsa_shm_put(map, "b", 2);
	apedsa_shm_put(map, "c", 3);
	ASSERT_EQ(apedsa_shm_len(map), 3);
	ASSERT_EQ(apedsa_shm_get(map, "a"), 1);
	ASSERT_EQ(apedsa_shm_get(map, "b"), 2);
	ASSERT_EQ(apedsa_shm_get(map, "c"), 3);
	apedsa_shm_del(map, "b");
	ASSERT_EQ(apedsa_shm_len(map), 2);
	ASSERT_EQ(apedsa_shm_get(map, "a"), 1);
	ASSERT_EQ(apedsa_shm_get(map, "c"), 3);
	return PASSED;
}

TEST(hm_string_delete_first)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "a", 1);
	apedsa_shm_put(map, "b", 2);
	apedsa_shm_put(map, "c", 3);
	apedsa_shm_del(map, "a");
	ASSERT_EQ(apedsa_shm_len(map), 2);
	ASSERT_EQ(apedsa_shm_get(map, "b"), 2);
	ASSERT_EQ(apedsa_shm_get(map, "c"), 3);
	return PASSED;
}

TEST(hm_string_delete_last)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "a", 1);
	apedsa_shm_put(map, "b", 2);
	apedsa_shm_put(map, "c", 3);
	apedsa_shm_del(map, "c");
	ASSERT_EQ(apedsa_shm_len(map), 2);
	ASSERT_EQ(apedsa_shm_get(map, "a"), 1);
	ASSERT_EQ(apedsa_shm_get(map, "b"), 2);
	return PASSED;
}

TEST(hm_string_nonexistent_key)
{
	Kv *map = NULL;
	apedsa_shm_put(map, "exists", 42);
	ASSERT_EQ(apedsa_shm_geti(map, "notexists"), -1);
	return PASSED;
}

TEST(hm_string_empty_map)
{
	Ki *map = NULL;
	ASSERT_EQ(apedsa_hm_len(map), 0);
	return PASSED;
}

TEST(hm_int_put_one)
{
	Ki *map = NULL;
	int k = 42;
	apedsa_hm_put(map, k, 100);
	ASSERT_EQ(apedsa_hm_get(map, k), 100);
	return PASSED;
}

TEST(hm_int_put_many)
{
	Ki *map = NULL;
	for (int i = 0; i < 100; i++) {
		apedsa_hm_put(map, i, i * 7);
	}
	for (int i = 0; i < 100; i++) {
		ASSERT_EQ(apedsa_hm_get(map, i), i * 7);
	}
	return PASSED;
}

TEST(hm_int_update_existing)
{
	Ki *map = NULL;
	int k = 10;
	apedsa_hm_put(map, k, 50);
	ASSERT_EQ(apedsa_hm_get(map, k), 50);
	apedsa_hm_put(map, k, 100);
	ASSERT_EQ(apedsa_hm_get(map, k), 100);
	return PASSED;
}

TEST(hm_int_delete)
{
	Ki *map = NULL;
	int k;
	k = 1, apedsa_hm_put(map, k, 10);
	k = 2, apedsa_hm_put(map, k, 20);
	k = 3, apedsa_hm_put(map, k, 30);
	ASSERT_EQ(apedsa_hm_len(map), 3);
	k = 2, apedsa_hm_del(map, k);
	ASSERT_EQ(apedsa_hm_len(map), 2);
	ASSERT_EQ((k = 1, apedsa_hm_get(map, k)), 10);
	ASSERT_EQ((k = 3, apedsa_hm_get(map, k)), 30);
	return PASSED;
}

TEST(hm_int_delete_first)
{
	Ki *map = NULL;
	int k;
	k = 1, apedsa_hm_put(map, k, 10);
	k = 2, apedsa_hm_put(map, k, 20);
	k = 3, apedsa_hm_put(map, k, 30);
	k = 1, apedsa_hm_del(map, k);
	ASSERT_EQ(apedsa_hm_len(map), 2);
	ASSERT_EQ((k = 2, apedsa_hm_get(map, k)), 20);
	ASSERT_EQ((k = 3, apedsa_hm_get(map, k)), 30);
	return PASSED;
}

TEST(hm_int_delete_last)
{
	Ki *map = NULL;
	int k;
	k = 1, apedsa_hm_put(map, k, 10);
	k = 2, apedsa_hm_put(map, k, 20);
	k = 3, apedsa_hm_put(map, k, 30);
	apedsa_hm_del(map, k);
	ASSERT_EQ(apedsa_hm_len(map), 2);
	ASSERT_EQ((k = 1, apedsa_hm_get(map, k)), 10);
	ASSERT_EQ((k = 2, apedsa_hm_get(map, k)), 20);
	return PASSED;
}

TEST(hm_int_nonexistent_key)
{
	Ki *map = NULL;
	int k = 100;
	apedsa_hm_put(map, k, 999);
	ASSERT_EQ((k = 999, apedsa_hm_geti(map, k)), -1);
	return PASSED;
}

TEST(hm_int_empty_map)
{
	Ki *map = NULL;
	ASSERT_EQ(apedsa_hm_len(map), 0);
	return PASSED;
}

TEST(hm_int_stress)
{
	Ki *map = NULL;
	int num_ops = 10000;
	for (int i = 0; i < num_ops; i++) {
		apedsa_hm_put(map, i, i * 3);
	}
	ASSERT_EQ(apedsa_hm_len(map), num_ops);
	for (int i = 0; i < num_ops; i++) {
		ASSERT_EQ(apedsa_hm_get(map, i), i * 3);
	}
	return PASSED;
}

TEST(hm_string_stress)
{
	Kv *map = NULL;
	int num_ops = 1000;
	for (int i = 0; i < num_ops; i++) {
		char key[30];
		sprintf(key, "stringkey-%d", i);
		apedsa_shm_put(map, key, i * 5);
	}
	ASSERT_EQ(apedsa_shm_len(map), num_ops);
	for (int i = 0; i < num_ops; i++) {
		char key[30];
		sprintf(key, "stringkey-%d", i);
		ASSERT_EQ(apedsa_shm_get(map, key), i * 5);
	}
	return PASSED;
}

TEST(hm_mixed_operations)
{
	Ki *imap = NULL;
	Kv *smap = NULL;
	for (int i = 0; i < 25; i++) {
		apedsa_hm_put(imap, i, i * 2);
		char key[20];
		sprintf(key, "key%d", i);
		apedsa_shm_put(smap, key, i * 3);
	}
	for (int i = 0; i < 25; i++) {
		char key[20];
		sprintf(key, "key%d", i);
		ASSERT_EQ(apedsa_hm_get(imap, i), i * 2);
		ASSERT_EQ(apedsa_shm_get(smap, key), i * 3);
	}
	int k = 10;
	apedsa_hm_del(imap, k);
	apedsa_shm_del(smap, "key10");
	ASSERT_EQ(apedsa_hm_len(imap), 24);
	ASSERT_EQ(apedsa_shm_len(smap), 24);
	return PASSED;
}

TEST(hm_reinsert_after_delete)
{
	Ki *map = NULL;
	int k;
	k = 1, apedsa_hm_put(map, k, 100);
	k = 2, apedsa_hm_put(map, k, 200);
	k = 3, apedsa_hm_put(map, k, 300);
	k = 2, apedsa_hm_del(map, k);
	ASSERT_EQ(apedsa_hm_len(map), 2);
	apedsa_hm_put(map, k, 250);
	ASSERT_EQ(apedsa_hm_len(map), 3);
	ASSERT_EQ((k = 1, apedsa_hm_get(map, k)), 100);
	ASSERT_EQ((k = 2, apedsa_hm_get(map, k)), 250);
	ASSERT_EQ((k = 3, apedsa_hm_get(map, k)), 300);
	return PASSED;
}

TEST(hm_put_batch)
{
	Ki *map = NULL;
	apedsa_hm_put_batch(map,
			    ((Ki[]){
				    { .key = 1, .value = 1 },
				    { .key = 2, .value = 2 },
				    { .key = 3, .value = 3 },
				    { .key = 4, .value = 4 },
				    { .key = 5, .value = 5 },
				    { .key = 6, .value = 6 },
				    { .key = 7, .value = 7 },
				    { .key = 8, .value = 8 },
				    { .key = 9, .value = 9 },
				    { .key = 10, .value = 10 },
			    }),
			    10);
	ASSERT_EQ(apedsa_hm_len(map), 10);
	int k;
	ASSERT_EQ((k = 1, apedsa_hm_get(map, k)), 1);
	ASSERT_EQ((k = 2, apedsa_hm_get(map, k)), 2);
	ASSERT_EQ((k = 3, apedsa_hm_get(map, k)), 3);
	ASSERT_EQ((k = 4, apedsa_hm_get(map, k)), 4);
	ASSERT_EQ((k = 5, apedsa_hm_get(map, k)), 5);
	ASSERT_EQ((k = 6, apedsa_hm_get(map, k)), 6);
	ASSERT_EQ((k = 7, apedsa_hm_get(map, k)), 7);
	ASSERT_EQ((k = 8, apedsa_hm_get(map, k)), 8);
	ASSERT_EQ((k = 9, apedsa_hm_get(map, k)), 9);
	ASSERT_EQ((k = 10, apedsa_hm_get(map, k)), 10);
	return PASSED;
}

TEST(shm_put_batch)
{
	Kv *map = NULL;
	Kv *da = NULL;
	apedsa_da_put(da, ((Kv){ .key = "a", .value = 1 }));
	apedsa_da_put(da, ((Kv){ .key = "b", .value = 2 }));
	apedsa_da_put(da, ((Kv){ .key = "c", .value = 3 }));
	apedsa_da_put(da, ((Kv){ .key = "d", .value = 4 }));
	apedsa_da_put(da, ((Kv){ .key = "e", .value = 5 }));
	apedsa_da_put(da, ((Kv){ .key = "f", .value = 6 }));
	apedsa_da_put(da, ((Kv){ .key = "g", .value = 7 }));
	apedsa_da_put(da, ((Kv){ .key = "h", .value = 8 }));
	apedsa_da_put(da, ((Kv){ .key = "i", .value = 9 }));
	apedsa_da_put(da, ((Kv){ .key = "j", .value = 10 }));

	apedsa_shm_put_batch(map, da, 10);
	ASSERT_EQ(apedsa_shm_len(map), 10);
	ASSERT_EQ(apedsa_shm_get(map, "a"), 1);
	ASSERT_EQ(apedsa_shm_get(map, "b"), 2);
	ASSERT_EQ(apedsa_shm_get(map, "c"), 3);
	ASSERT_EQ(apedsa_shm_get(map, "d"), 4);
	ASSERT_EQ(apedsa_shm_get(map, "e"), 5);
	ASSERT_EQ(apedsa_shm_get(map, "f"), 6);
	ASSERT_EQ(apedsa_shm_get(map, "g"), 7);
	ASSERT_EQ(apedsa_shm_get(map, "h"), 8);
	ASSERT_EQ(apedsa_shm_get(map, "i"), 9);
	ASSERT_EQ(apedsa_shm_get(map, "j"), 10);
	return PASSED;
}

typedef struct {
	char *key;
	char *value;
} Kvss;

TEST(shm_string_values)
{
	Kvss *map = NULL;
	apedsa_shm_put(map, "hello", "world");
	ASSERT_STR_EQ(apedsa_shm_get(map, "hello"), "world");
	return PASSED;
}

static void run_hm_tests(void)
{
	LOG_INFO("HM tests:");
	RUN_TEST(hm_string_put_one);
	RUN_TEST(hm_string_put_many);
	RUN_TEST(hm_string_update_existing);
	RUN_TEST(hm_string_delete);
	RUN_TEST(hm_string_delete_first);
	RUN_TEST(hm_string_delete_last);
	RUN_TEST(hm_string_nonexistent_key);
	RUN_TEST(hm_string_empty_map);
	RUN_TEST(hm_int_put_one);
	RUN_TEST(hm_int_put_many);
	RUN_TEST(hm_int_update_existing);
	RUN_TEST(hm_int_delete);
	RUN_TEST(hm_int_delete_first);
	RUN_TEST(hm_int_delete_last);
	RUN_TEST(hm_int_nonexistent_key);
	RUN_TEST(hm_int_empty_map);
	RUN_TEST(hm_int_stress);
	RUN_TEST(hm_string_stress);
	RUN_TEST(hm_mixed_operations);
	RUN_TEST(hm_reinsert_after_delete);
	RUN_TEST(hm_put_batch);
	RUN_TEST(shm_put_batch);
	RUN_TEST(shm_string_values);
}

int main(void)
{
	LOG_INFO("Running tests...");
	run_da_tests();
	run_hm_tests();
	LOG_INFO("Tests finished");
	LOG_INFO("%d Total", tests_run);
	if (tests_failed > 0)
		LOG_INFO("%d \x1b[31mFAILED\x1b[0m", tests_failed);
	if (tests_passed > 0)
		LOG_INFO("%d \x1b[32mPASSED\x1b[0m", tests_passed);
	return tests_failed > 0 ? 1 : 0;
}
