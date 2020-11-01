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

void assertNullableStringEquals(NullableString* actual, NullableString* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assertStrEquals(actual->str, expected->str);
  }
}

void assertDataMessageEquals(DataMessage* actual, DataMessage* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(actual->data.len == expected->data.len);
    assertEquals(actual->data.data, expected->data.data, expected->data.len);
  }
}

void assertEntryMessage(EntryMessage* actual, EntryMessage* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assertNullableStringEquals(actual->key, expected->key);
    assertDataMessageEquals(actual->value, expected->value);
  }
}

void assertKeysMessage(KeysMessage* actual, KeysMessage* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(actual->n_keys == expected->n_keys);
    for (int i = 0; i < expected->n_keys; ++i) {
      assertNullableStringEquals(actual->keys[i], expected->keys[i]);
    }
  }
}

void assertMessageEquals(Message* actual, Message* expected) {
  if (expected == NULL) {
    assert(actual == NULL);
  } else {
    assert(actual != NULL);
    assert(actual->op_code == expected->op_code);
    assert(actual->content_case == expected->content_case);
    switch (expected->content_case) {
      case CT_KEY:
        assertNullableStringEquals(actual->key, expected->key);
        break;
      case CT_VALUE:
        assertDataMessageEquals(actual->value, expected->value);
        break;
      case CT_ENTRY:
        assertEntryMessage(actual->entry, expected->entry);
        break;
      case CT_KEYS:
        assertKeysMessage(actual->keys, expected->keys);
        break;
      case CT_INT_RESULT:
        assert(actual->int_result == expected->int_result);
        break;
      default:
        break;
    }
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
