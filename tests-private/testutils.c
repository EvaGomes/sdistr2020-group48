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
  assert(strcmp(actual, expected) == 0);
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
