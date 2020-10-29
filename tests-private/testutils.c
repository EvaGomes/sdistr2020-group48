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
    assert(strcmp(actual, expected) == 0);
  }
}

void assertEquals(void* actual, void* expected, int size) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
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

void assertNodeHasKey(struct tree_node_t* actualNode, char* expectedKey) {
  assertStrEquals(actualNode->entry->key, expectedKey);
}

void assertNodeHas(struct tree_node_t* actualNode, char* expectedKey, char* expectedValueData) {
  assertNodeHasKey(actualNode, expectedKey);
  assert(actualNode->entry->value != NULL);
  assert(actualNode->entry->value->data != NULL);
  assertStrEquals(actualNode->entry->value->data, expectedValueData);
}
