#include "data.h"
#include "entry.h"
#include "serialization.h"
#include "tree-private.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./testutils.c"

// **************************************************************
// data.c
// **************************************************************

void test__data_create__datasize_0() {
  printTestIntro("data.c", "data_create datasize=0");

  struct data_t* dataStruct = data_create(0);
  assert(dataStruct != NULL);
  assert(dataStruct->datasize == 0);
  assert(dataStruct->data == NULL);

  data_destroy(dataStruct);
  printTestDone();
}

void test__data_create2__datasize_0__data_NULL() {
  printTestIntro("data.c", "data_create2 datasize=0 data=NULL");

  struct data_t* dataStruct = data_create2(0, NULL);
  assert(dataStruct != NULL);
  assert(dataStruct->datasize == 0);
  assert(dataStruct->data == NULL);

  data_destroy(dataStruct);
  printTestDone();
}

void test__data_create2__datasize_not0__data_NULL() {
  printTestIntro("data.c", "data_create2 datasize=5 data=NULL");

  struct data_t* dataStruct = data_create2(5, NULL);
  assert(dataStruct == NULL);

  printTestDone();
}

void test__data_dup__datasize_0__data_NULL() {
  printTestIntro("data.c", "data_dup datasize=0 data=NULL");

  struct data_t* dataStruct = data_create2(0, NULL);
  struct data_t* copiedDataStruct = data_dup(dataStruct);
  assert(copiedDataStruct != NULL);
  assert(copiedDataStruct->datasize == 0);
  assert(copiedDataStruct->data == NULL);

  data_destroy(copiedDataStruct);
  data_destroy(dataStruct);
  printTestDone();
}

void test__data_replace__datasize_0__data_NULL() {
  printTestIntro("data.c", "data_replace datasize=0 data=NULL");

  char* data = strdup("abc");
  struct data_t* dataStruct = data_create2(strlen(data) + 1, data);
  assert(dataStruct != NULL);
  assert(dataStruct->datasize == strlen(data) + 1);
  assert(dataStruct->data == data);

  data_replace(dataStruct, 0, NULL);
  assert(dataStruct->datasize == 0);
  assert(dataStruct->data == NULL);

  data_destroy(dataStruct);
  printTestDone();
}

// **************************************************************
// entry.c
// **************************************************************

void test__entry_create__NULL_key_NULL_value() {
  printTestIntro("entry.c", "entry_create key=NULL value=NULL");

  char* key = NULL;
  struct data_t* value = NULL;
  struct entry_t* entry = entry_create(key, value);

  assert(entry->key == key);
  assert(entry->value == value);

  free(entry);

  printTestDone();
}

void test__entry_destroy__NULL_key_NULL_value() {
  printTestIntro("entry.c", "entry_destroy key=NULL value=NULL");

  char* key = NULL;
  struct data_t* value = NULL;
  struct entry_t* entry = entry_create(key, value);

  entry_destroy(entry);

  printTestDone();
}

void test__entry_dup__NULL_key_NULL_value() {
  printTestIntro("entry.c", "entry_dup key=NULL value=NULL");

  char* key = NULL;
  struct data_t* value = NULL;
  struct entry_t* entry = entry_create(key, value);
  struct entry_t* copied_entry = entry_dup(entry);

  assert(copied_entry != entry);
  assert(copied_entry->key == entry->key);
  assert(copied_entry->value == entry->value);

  entry_destroy(entry);
  entry_destroy(copied_entry);

  printTestDone();
}

void test__entry_replace__NULL_key_NULL_value() {
  printTestIntro("entry.c", "entry_replace new_key=NULL new_value=NULL");

  char* key = strdup("123abc");
  char* data = strdup("1234567890abc");
  struct data_t* value = data_create2(strlen(data) + 1, data);
  struct entry_t* entry = entry_create(key, value);

  char* new_key = NULL;
  struct data_t* new_value = NULL;
  entry_replace(entry, new_key, new_value);

  assert(entry->key == new_key);
  assert(entry->value == new_value);

  entry_destroy(entry);

  printTestDone();
}

void test__entry_compare__NULL_entries() {
  printTestIntro("entry.c", "entry_compare with NULL entries");

  char* key = strdup("a");
  char* data = strdup("1234567890abc");
  struct data_t* value = data_create2(strlen(data) + 1, data);
  struct entry_t* entry = entry_create(key, value);

  assert(entry_compare(entry, NULL) == +1);
  assert(entry_compare(NULL, entry) == -1);
  assert(entry_compare(NULL, NULL) == 0);

  entry_destroy(entry);

  printTestDone();
}

void test__entry_compare__NULL_keys() {
  printTestIntro("entry.c", "entry_compare with NULL keys");

  char* key1 = strdup("a");
  struct data_t* value1 = data_create2(strlen("1") + 1, strdup("1"));
  struct entry_t* entry1 = entry_create(key1, value1);

  char* key2 = NULL;
  struct data_t* value2 = data_create2(strlen("2") + 1, strdup("2"));
  struct entry_t* entry2 = entry_create(key2, value2);

  char* key3 = NULL;
  struct data_t* value3 = data_create2(strlen("3") + 1, strdup("3"));
  struct entry_t* entry3 = entry_create(key3, value3);

  assert(entry_compare(entry1, entry2) == +1);
  assert(entry_compare(entry2, entry1) == -1);
  assert(entry_compare(entry2, entry2) == 0);
  assert(entry_compare(entry2, entry3) == 0);

  entry_destroy(entry1);
  entry_destroy(entry2);
  entry_destroy(entry3);

  printTestDone();
}

// **************************************************************
// tree.c
// **************************************************************

struct tree_t* _createTreeWith7Nodes() {
  struct tree_t* tree = tree_create();

  int labels[] = {4, 3, 2, 1, 8, 9, 5};
  for (int i = 0; i < 7; ++i) {
    char key[5];
    sprintf(key, "key%d", labels[i]);
    char data[7];
    sprintf(data, "value%d", labels[i]);
    struct data_t* value = data_create2(strlen(data) + 1, data);
    tree_put(tree, key, value);

    free(value);
  }

  return tree;
}

void test__tree_with_7_nodes() {
  printTestIntro("tree.c", "create and operate tree with 7 nodes");

  struct tree_t* tree = _createTreeWith7Nodes();

  assert(tree != NULL);
  assert(tree->size == 7);
  assert(tree->height == 3);
  assert(tree->root != NULL);

  assertNodeHasKey(tree->root, "key4");
  assertNodeHasKey(tree->root->left, "key3");
  assertNodeHasKey(tree->root->left->left, "key2");
  assertNodeHasKey(tree->root->left->left->left, "key1");
  assert(tree->root->left->left->right == NULL);
  assert(tree->root->left->right == NULL);
  assertNodeHasKey(tree->root->right, "key8");
  assertNodeHasKey(tree->root->right->left, "key5");
  assert(tree->root->right->left->left == NULL);
  assert(tree->root->right->left->right == NULL);
  assertNodeHasKey(tree->root->right->right, "key9");
  assert(tree->root->right->right->left == NULL);
  assert(tree->root->right->right->right == NULL);

  tree_destroy(tree);
  printTestDone();
}

void test__tree_get__unexistent_key() {
  printTestIntro("tree.c", "tree_get unexistent key");

  struct tree_t* tree = _createTreeWith7Nodes();
  struct data_t* result = tree_get(tree, "unexistent key");

  assert(result != NULL);
  assert(result->datasize == 0);
  assert(result->data == NULL);

  data_destroy(result);
  tree_destroy(tree);
  printTestDone();
}

void test__tree_del__existent_key() {
  printTestIntro("tree.c", "tree_del existent key");

  struct tree_t* tree = _createTreeWith7Nodes();
  int result = tree_del(tree, "key3");

  assert(result == 0);
  assert(tree->size == 6);
  struct data_t* resultAfterDeletion = tree_get(tree, "key3");
  assert(resultAfterDeletion->datasize == 0);
  assert(resultAfterDeletion->data == NULL);

  data_destroy(resultAfterDeletion);
  tree_destroy(tree);
  printTestDone();
}

// **************************************************************
// serialization.c
// **************************************************************

void test__data_serialization() {
  printTestIntro("serialization.c", "data_to_buffer and buffer_to_data");

  char* data = strdup("1234567890abc");
  int datasize = strlen(data) + 1;
  struct data_t* dataStruct = data_create2(datasize, data);

  char* data_buf;
  int len_data_buf = data_to_buffer(dataStruct, &data_buf);
  assert(len_data_buf >= 0);
  assert(len_data_buf == 2 + 14);
  assert(data_buf != NULL);

  struct data_t* deserialized = buffer_to_data(data_buf, len_data_buf);
  assertDataEquals(deserialized, dataStruct);

  data_destroy(deserialized);
  free(data_buf);
  data_destroy(dataStruct);
  printTestDone();
}

void test__data_serialization__NULL_data() {
  printTestIntro("serialization.c", "data_to_buffer and buffer_to_data with NULL data");

  struct data_t* dataStruct = data_create(0);

  char* data_buf;
  int len_data_buf = data_to_buffer(dataStruct, &data_buf);
  assert(len_data_buf >= 0);
  assert(len_data_buf == 0);
  assert(data_buf != NULL);

  struct data_t* deserialized = buffer_to_data(data_buf, len_data_buf);
  assertDataEquals(deserialized, dataStruct);

  data_destroy(deserialized);
  free(data_buf);
  data_destroy(dataStruct);
  printTestDone();
}

void test__data_serialization__big_data() {
  printTestIntro("serialization.c", "data_to_buffer and buffer_to_data with biiiiig data");

  int count = 5000;
  int datasize = sizeof(long) * count;
  long* data = malloc(datasize);
  for (int i = 0; i < count; ++i) {
    data[i] = 5 * (500000000 + i);
  }
  struct data_t* dataStruct = data_create2(datasize, data);

  char* data_buf;
  int len_data_buf = data_to_buffer(dataStruct, &data_buf);
  assert(len_data_buf >= 0);
  assert(len_data_buf == 4 + 40000);
  assert(data_buf != NULL);

  struct data_t* deserialized = buffer_to_data(data_buf, len_data_buf);
  assertDataEquals(deserialized, dataStruct);

  data_destroy(deserialized);
  free(data_buf);
  data_destroy(dataStruct);
  printTestDone();
}

void test__entry_serialization() {
  printTestIntro("serialization.c", "entry_to_buffer and buffer_to_entry");

  char* key = strdup("leKey");
  char* data = strdup("1234567890abc");
  int datasize = strlen(data) + 1;
  struct data_t* value = data_create2(datasize, data);
  struct entry_t* entry = entry_create(key, value);

  char* serialized;
  int len_serialized = entry_to_buffer(entry, &serialized);
  assert(len_serialized >= 0);
  assert(len_serialized == 25);
  assert(serialized != NULL);

  struct entry_t* deserialized = buffer_to_entry(serialized, len_serialized);
  assert(deserialized != NULL);
  assertStrEquals(deserialized->key, key);
  assertDataEquals(deserialized->value, value);

  entry_destroy(deserialized);
  free(serialized);
  entry_destroy(entry);
  printTestDone();
}

void test__entry_serialization__NULL_key() {
  printTestIntro("serialization.c", "entry_to_buffer and buffer_to_entry with NULL key");

  char* data = strdup("1234567890abc");
  int datasize = strlen(data) + 1;
  struct data_t* value = data_create2(datasize, data);
  struct entry_t* entry = entry_create(NULL, value);

  char* serialized;
  int len_serialized = entry_to_buffer(entry, &serialized);
  assert(len_serialized >= 0);
  assert(len_serialized == 18);
  assert(serialized != NULL);

  struct entry_t* deserialized = buffer_to_entry(serialized, len_serialized);
  assert(deserialized != NULL);
  assert(deserialized->key == NULL);
  assertDataEquals(deserialized->value, value);

  entry_destroy(deserialized);
  free(serialized);
  entry_destroy(entry);
  printTestDone();
}

void test__entry_serialization__NULL_value() {
  printTestIntro("serialization.c", "entry_to_buffer and buffer_to_entry with NULL value");

  char* key = strdup("leKey");
  struct entry_t* entry = entry_create(key, NULL);

  char* serialized;
  int len_serialized = entry_to_buffer(entry, &serialized);
  assert(len_serialized >= 0);
  assert(len_serialized == 7);
  assert(serialized != NULL);

  struct entry_t* deserialized = buffer_to_entry(serialized, len_serialized);
  assert(deserialized != NULL);
  assertStrEquals(deserialized->key, key);
  assert(deserialized->value == NULL);

  entry_destroy(deserialized);
  free(serialized);
  entry_destroy(entry);
  printTestDone();
}

void test__entry_serialization__NULL_data() {
  printTestIntro("serialization.c", "entry_to_buffer and buffer_to_entry with NULL data");

  char* key = strdup("leKey");
  struct data_t* value = data_create2(0, NULL);
  struct entry_t* entry = entry_create(key, value);

  char* serialized;
  int len_serialized = entry_to_buffer(entry, &serialized);
  assert(len_serialized >= 0);
  assert(len_serialized == 9);
  assert(serialized != NULL);

  struct entry_t* deserialized = buffer_to_entry(serialized, len_serialized);
  assert(deserialized != NULL);
  assertStrEquals(deserialized->key, key);
  assertDataEquals(deserialized->value, value);

  entry_destroy(deserialized);
  free(serialized);
  entry_destroy(entry);
  printTestDone();
}

void test__entry_serialization__NULL_key_and_NULL_value() {
  printTestIntro("serialization.c", "entry_to_buffer and buffer_to_entry with NULL value");

  struct entry_t* entry = entry_create(NULL, NULL);

  char* serialized;
  int len_serialized = entry_to_buffer(entry, &serialized);
  assert(len_serialized >= 0);
  assert(len_serialized == 0);
  assert(serialized != NULL);

  struct entry_t* deserialized = buffer_to_entry(serialized, len_serialized);
  assert(deserialized != NULL);
  assert(deserialized->key == NULL);
  assert(deserialized->value == NULL);

  entry_destroy(deserialized);
  free(serialized);
  entry_destroy(entry);
  printTestDone();
}

void test__tree_serialization() {
  printTestIntro("serialization.c", "tree_to_buffer and buffer_to_tree");

  struct tree_t* tree = _createTreeWith7Nodes();
  char* serialized_tree;
  int len_serialized_tree = tree_to_buffer(tree, &serialized_tree);
  struct tree_t* deserialized_tree = buffer_to_tree(serialized_tree, len_serialized_tree);

  assert(deserialized_tree != NULL);
  assert(deserialized_tree->size == 7);
  assert(deserialized_tree->height == 3);
  assert(deserialized_tree->root != NULL);

  assertNodeHas(deserialized_tree->root, "key4", "value4");
  assertNodeHas(deserialized_tree->root->left, "key3", "value3");
  assertNodeHas(deserialized_tree->root->left->left, "key2", "value2");
  assertNodeHas(deserialized_tree->root->left->left->left, "key1", "value1");
  assert(deserialized_tree->root->left->left->right == NULL);
  assert(deserialized_tree->root->left->right == NULL);
  assertNodeHas(deserialized_tree->root->right, "key8", "value8");
  assertNodeHas(deserialized_tree->root->right->left, "key5", "value5");
  assert(deserialized_tree->root->right->left->left == NULL);
  assert(deserialized_tree->root->right->left->right == NULL);
  assertNodeHas(deserialized_tree->root->right->right, "key9", "value9");
  assert(deserialized_tree->root->right->right->left == NULL);
  assert(deserialized_tree->root->right->right->right == NULL);

  tree_destroy(deserialized_tree);
  free(serialized_tree);
  tree_destroy(tree);

  printTestDone();
}

// **************************************************************

int main() {

  test__data_create__datasize_0();
  test__data_create2__datasize_0__data_NULL();
  test__data_create2__datasize_not0__data_NULL();
  test__data_dup__datasize_0__data_NULL();
  test__data_replace__datasize_0__data_NULL();

  test__entry_create__NULL_key_NULL_value();
  test__entry_destroy__NULL_key_NULL_value();
  test__entry_dup__NULL_key_NULL_value();
  test__entry_replace__NULL_key_NULL_value();
  test__entry_compare__NULL_entries();
  test__entry_compare__NULL_keys();

  test__tree_with_7_nodes();
  test__tree_get__unexistent_key();
  test__tree_del__existent_key();

  test__data_serialization();
  test__data_serialization__NULL_data();
  test__data_serialization__big_data();
  test__entry_serialization();
  test__entry_serialization__NULL_key();
  test__entry_serialization__NULL_value();
  test__entry_serialization__NULL_data();
  test__entry_serialization__NULL_key_and_NULL_value();
  test__tree_serialization();

  printf("\n\nDONE: No assertions failed!\n");
  return 0;
}