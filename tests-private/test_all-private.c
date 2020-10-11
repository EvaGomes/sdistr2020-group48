#include "../utils.c"
#include "data.h"
#include "entry.h"
#include "serialization.h"
#include "tree-private.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// **************************************************************
// testing utils
// **************************************************************

void printTestIntro(char* moduleName, char* testName) {
  printf("\n%s -- %s", moduleName, testName);
}

void pee(const char* msg) {
  perror(msg);
  exit(0);
}

void printTestDone() {
  printf(" --  PASSED");
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

void test__tree_with_7_nodes() {
  printTestIntro("tree.c", "create and operate tree");

  struct tree_t* tree = tree_create();

  int labels[] = {4, 3, 2, 1, 8, 9, 5};
  const int num_of_nodes = sizeof(labels) / sizeof(int);

  for (int i = 0; i < num_of_nodes; ++i) {
    char key[5];
    sprintf(key, "key%d", labels[i]);
    char data[7];
    sprintf(data, "value%d", labels[i]);
    struct data_t* value = data_create2(strlen(data) + 1, data);

    tree_put(tree, key, value);

    free(value);
  }

  assert(tree != NULL);
  assert(tree->size == num_of_nodes);
  assert(tree->height == 3);
  assert(tree->root != NULL);

  assert(strcmp(tree->root->entry->key, "key4") == 0);
  assert(strcmp(tree->root->left->entry->key, "key3") == 0);
  assert(strcmp(tree->root->left->left->entry->key, "key2") == 0);
  assert(strcmp(tree->root->left->left->left->entry->key, "key1") == 0);
  assert(tree->root->left->left->right == NULL);
  assert(tree->root->left->right == NULL);
  assert(strcmp(tree->root->right->entry->key, "key8") == 0);
  assert(strcmp(tree->root->right->left->entry->key, "key5") == 0);
  assert(tree->root->right->left->left == NULL);
  assert(tree->root->right->left->right == NULL);
  assert(strcmp(tree->root->right->right->entry->key, "key9") == 0);
  assert(tree->root->right->right->left == NULL);
  assert(tree->root->right->right->right == NULL);

  tree_destroy(tree);

  printTestDone();
}

// **************************************************************

int main() {
  printf("-------------------------\n");
  printf("test_all-private\n");
  printf("-------------------------\n");

  test__entry_create__NULL_key_NULL_value();
  test__entry_destroy__NULL_key_NULL_value();
  test__entry_dup__NULL_key_NULL_value();
  test__entry_replace__NULL_key_NULL_value();
  test__entry_compare__NULL_entries();
  test__entry_compare__NULL_keys();

  test__tree_with_7_nodes();

  printf("\n\nDONE: No assertions failed!\n");
  printf("-------------------------\n");
  return 0;
}