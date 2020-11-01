#include "tree-private.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
// assertions
// **************************************************************

void assertStrEquals(char* actual, char* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(strcmp(actual, expected) == 0);
  }
}

void assertEquals(void* actual, void* expected, int size) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(memcmp(actual, expected, size) == 0);
  }
}

void assertDataEquals(struct data_t* actual, struct data_t* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(actual->datasize == expected->datasize);
    assertEquals(actual->data, expected->data, expected->datasize);
  }
}

void assertEntryEquals(struct entry_t* actual, struct entry_t* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assertStrEquals(actual->key, expected->key);
    assertDataEquals(actual->value, expected->value);
  }
}

void assertStrArrEquals(char** actual, char** expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    int current_index = 0;
    while (expected[current_index] != NULL) {
      assertStrEquals(actual[current_index], expected[current_index]);
      current_index += 1;
    }
    assert(actual[current_index] == NULL);
  }
}

void assertNodeHasKey(struct tree_node_t* actualNode, char* expectedKey) {
  assert(actualNode != NULL);
  assertStrEquals(actualNode->entry->key, expectedKey);
}

void assertNodeHasKeyAndNullValue(struct tree_node_t* actualNode, char* expectedKey) {
  assert(actualNode != NULL);
  assertNodeHasKey(actualNode, expectedKey);
  assert(actualNode->entry->value == NULL);
}

void assertNodeHas(struct tree_node_t* actualNode, char* expectedKey, char* expectedValueData) {
  assert(actualNode != NULL);
  assertNodeHasKey(actualNode, expectedKey);
  assert(actualNode->entry->value != NULL);
  assertStrEquals(actualNode->entry->value->data, expectedValueData);
}
