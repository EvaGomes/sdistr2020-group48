#include "logger-private.h"
#include "message-private.h"
#include "tasks-private.h"
#include "tree_skel.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./testutils.c"

struct message_t* _message_t_create(Message__OperationCode op_code,
                                    Message__ContentCase content_case) {
  struct message_t* message = message_create();
  message->msg->op_code = op_code;
  message->msg->content_case = content_case;
  return message;
}

struct entry_t* _entry_t_create(int n) {
  char* key = malloc(5 * sizeof(char));
  char* value = malloc(7 * sizeof(char));
  sprintf(key, "key%d", n);
  sprintf(value, "value%d", n);
  return entry_create(key, data_create2(7, value));
}

void _assertUnknownMessage(Message__OperationCode op_code, Message__ContentCase content_case) {
  struct message_t* message = _message_t_create(op_code, content_case);
  assert(invoke(message) == 0);
  assertMessageHasIntResult(message, OP_ERROR, -1);
  message_destroy(message);
}

// **************************************************************
// tasks-private.c
// **************************************************************

void test__tasks() {
  printTestIntro("tasks-private.c", "normal flow");

  Message* fake_op0 = Message_create();
  fake_op0->op_code = OP_PUT;
  Message* fake_op1 = Message_create();
  fake_op1->op_code = OP_DEL;

  assert(tasks_init() >= 0);

  assert(tasks_set_result(0, SUCCESSFUL) < 0); // results are reject if no task was queued

  tasks_add_task(fake_op0);
  tasks_add_task(fake_op1);

  struct task_t* task0 = tasks_get_next();
  int task0_id = task0->task_id;
  assert(task0 != NULL);
  assert(task0_id >= 0);                                  // has valid id
  assertMessageEquals(task0->op_code_and_args, fake_op0); // has expected op
  assert(task0->op_code_and_args != fake_op0);            // but it is a copy of the given one
  assert(tasks_get_next() == task0); // returns same task until its result gets set

  assert(tasks_set_result(task0_id, NOT_EXECUTED) < 0); // invalid argument
  assert(tasks_get_result(task0_id) == NOT_EXECUTED);
  assert(tasks_set_result(task0_id, SUCCESSFUL) >= 0);
  assert(tasks_get_result(task0_id) == SUCCESSFUL);

  struct task_t* task1 = tasks_get_next();
  int task1_id = task1->task_id;
  assert(task1 != NULL);
  assert(task1_id >= task0_id);
  assertMessageEquals(task1->op_code_and_args, fake_op1); // has expected op
  assert(task1->op_code_and_args != fake_op1);            // but it is a copy of the given one

  // setting result of task_id_0 out of order, gets rejected
  assert(tasks_set_result(task0_id, SUCCESSFUL) < 0);

  // setting result of task_id_1 twice fails on the second attempt
  assert(tasks_set_result(task1_id, FAILED) >= 0);
  assert(tasks_get_result(task1_id) == FAILED);
  assert(tasks_set_result(task1_id, SUCCESSFUL) < 0);
  assert(tasks_get_result(task1_id) == FAILED);

  // assert(tasks_get_next() == NULL); // no tasks left, would be blocked waiting for next task

  assert(tasks_add_task(fake_op0) >= 0);   // accepts another task, equal to the first one
  struct task_t* task2 = tasks_get_next(); // returns that as the next task
  int task2_id = task2->task_id;
  assert(task2 != NULL);
  assert(task2_id >= task1_id);
  assertMessageEquals(task2->op_code_and_args, fake_op0);
  assert(task2->op_code_and_args != fake_op1);
  assert(tasks_set_result(task2_id, SUCCESSFUL) >= 0);

  // should be able to retrieve same results several times
  assert(tasks_get_result(task0_id) == SUCCESSFUL);
  assert(tasks_get_result(task1_id) == FAILED);
  assert(tasks_get_result(task2_id) == SUCCESSFUL);

  // assert(tasks_get_next() == NULL); // no tasks left, would be blocked waiting for next task

  assert(tasks_add_task(fake_op0) >= 0); // accepts another task, equal to the first one

  tasks_destroy(); // should clear all tasks and results
  assert(tasks_get_next() == NULL);
  assert(tasks_get_result(task0_id) == NOT_EXECUTED);

  Message_destroy(fake_op0);
  Message_destroy(fake_op1);

  printTestDone();
}

// **************************************************************
// tree_skel.c
// **************************************************************

void test__tree_skel__invoke__tree_not_initialized() {
  printTestIntro("tree_skel.c", "invoke fails if tree_skel_init not called");

  struct message_t* message = _message_t_create(OP_SIZE, MESSAGE__CONTENT__NOT_SET);
  assert(invoke(message) == -1);
  message_destroy(message);

  printTestDone();
}

void test__tree_skel__invoke__NULL_message_t() {
  printTestIntro("tree_skel.c", "invoke fails if message_t is NULL");

  tree_skel_init();
  assert(invoke(NULL) == -1);
  tree_skel_destroy();

  printTestDone();
}

void test__tree_skel__invoke__NULL_Message() {
  printTestIntro("tree_skel.c", "invoke fails if Message is NULL");
  tree_skel_init();

  struct message_t* message = malloc(SIZE_OF_MESSAGE);
  message->msg = NULL;
  assert(invoke(message) == -1);
  message_destroy(message);

  tree_skel_destroy();
  printTestDone();
}

void test__tree_skel__invoke__unknown_message() {
  printTestIntro("tree_skel.c", "invoke fails if combo (op_code + c_type) is unknown");

  tree_skel_init();

  _assertUnknownMessage(OP_BAD, CT_KEY);
  _assertUnknownMessage(OP_BAD, CT_VALUE);
  _assertUnknownMessage(OP_BAD, CT_ENTRY);
  _assertUnknownMessage(OP_BAD, CT_KEYS);
  _assertUnknownMessage(OP_BAD, CT_INT_RESULT);
  _assertUnknownMessage(OP_BAD, CT_NONE);

  _assertUnknownMessage(OP_DEL, CT_VALUE);
  _assertUnknownMessage(OP_DEL, CT_ENTRY);
  _assertUnknownMessage(OP_DEL, CT_KEYS);
  _assertUnknownMessage(OP_DEL, CT_INT_RESULT);
  _assertUnknownMessage(OP_DEL, CT_NONE);

  _assertUnknownMessage(OP_GET, CT_VALUE);
  _assertUnknownMessage(OP_GET, CT_ENTRY);
  _assertUnknownMessage(OP_GET, CT_KEYS);
  _assertUnknownMessage(OP_GET, CT_INT_RESULT);
  _assertUnknownMessage(OP_GET, CT_NONE);

  _assertUnknownMessage(OP_PUT, CT_KEY);
  _assertUnknownMessage(OP_PUT, CT_VALUE);
  _assertUnknownMessage(OP_PUT, CT_KEYS);
  _assertUnknownMessage(OP_PUT, CT_INT_RESULT);
  _assertUnknownMessage(OP_PUT, CT_NONE);

  _assertUnknownMessage(OP_ERROR, CT_KEY);
  _assertUnknownMessage(OP_ERROR, CT_VALUE);
  _assertUnknownMessage(OP_ERROR, CT_ENTRY);
  _assertUnknownMessage(OP_ERROR, CT_KEYS);
  _assertUnknownMessage(OP_ERROR, CT_INT_RESULT);
  _assertUnknownMessage(OP_ERROR, CT_NONE);

  tree_skel_destroy();

  printTestDone();
}

void _test_size(int expected_size) {
  struct message_t* message = _message_t_create(OP_SIZE, CT_NONE);
  assert(invoke(message) == 0);
  assertMessageHasIntResult(message, OP_SIZE + 1, expected_size);
  message_destroy(message);
}

void _test_height(int expected_height) {
  struct message_t* message = _message_t_create(OP_HEIGHT, CT_NONE);
  assert(invoke(message) == 0);
  assertMessageHasIntResult(message, OP_HEIGHT + 1, expected_height);
  message_destroy(message);
}

void _test_del(const char* key, int expected_result) {
  struct message_t* del_message = _message_t_create(OP_DEL, CT_KEY);
  del_message->msg->key = strdup(key);
  assert(invoke(del_message) == 0);
  assertMessageHasOpId(del_message, OP_DEL + 1);

  struct message_t* verify_message = message_create();
  for (int i = 0; i < 10; ++i) {
    verify_message->msg->op_code = OP_VERIFY;
    verify_message->msg->content_case = CT_OP_ID;
    verify_message->msg->op_id = del_message->msg->op_id;
    assert(invoke(verify_message) == 0);

    assert(verify_message->msg != NULL);
    assert(verify_message->msg->op_code == OP_VERIFY + 1);
    if (verify_message->msg->int_result == expected_result) {
      break;
    } else {
      logger_debug(
          "(attempt#%d) del verify_message->msg->int_result is still %d, trying again...\n", i,
          verify_message->msg->int_result);
      sleep(3);
    }
  }
  assert(verify_message->msg->int_result == expected_result);

  message_destroy(del_message);
  message_destroy(verify_message);
}

void _test_get(const char* key, char* expected_value) {
  struct message_t* message = _message_t_create(OP_GET, CT_KEY);
  message->msg->key = strdup(key);
  assert(invoke(message) == 0);
  assertMessageHasValue(message, OP_GET + 1, expected_value);
  message_destroy(message);
}

void _test_put_entry(struct entry_t* entry) {
  struct message_t* put_message = _message_t_create(OP_PUT, CT_ENTRY);
  put_message->msg->entry = entry_to_msg(entry);
  assert(invoke(put_message) == 0);
  assertMessageHasOpId(put_message, OP_PUT + 1);

  struct message_t* verify_message = message_create();
  for (int i = 0; i < 10; ++i) {
    verify_message->msg->op_code = OP_VERIFY;
    verify_message->msg->content_case = CT_OP_ID;
    verify_message->msg->op_id = put_message->msg->op_id;
    assert(invoke(verify_message) == 0);
    assert(verify_message->msg != NULL);
    assert(verify_message->msg->op_code == OP_VERIFY + 1);
    assert(verify_message->msg->content_case == CT_INT_RESULT);
    if (verify_message->msg->int_result == +1) {
      break;
    } else {
      logger_debug(
          "(attempt#%d) put verify_message->msg->int_result is still %d, trying again...\n", i,
          verify_message->msg->int_result);
      sleep(3);
    }
  }
  assert(verify_message->msg->int_result == +1);

  message_destroy(put_message);
  message_destroy(verify_message);
}

void _test_getkeys(int expected_n_keys, char** expected_keys) {
  struct message_t* actual_message = _message_t_create(OP_GETKEYS, CT_NONE);
  assert(invoke(actual_message) == 0);

  assert(actual_message->msg != NULL);
  assert(actual_message->msg->op_code == OP_GETKEYS + 1);
  assert(actual_message->msg->content_case == CT_KEYS);
  assert(actual_message->msg->keys != NULL);
  assert(actual_message->msg->keys->n_keys == expected_n_keys);
  char** actual_keys = actual_message->msg->keys->keys;
  if (expected_keys == NULL) {
    assert(actual_keys == NULL);
  } else {
    assert(actual_keys != NULL);
    for (int i = 0; i < expected_n_keys; ++i) {
      assertStrEquals(actual_keys[i], expected_keys[i]);
    }
  }

  message_destroy(actual_message);
}

void test__tree_skel__invoke() {
  printTestIntro("tree_skel.c", "invoke normal flow");
  tree_skel_init();

  // query tree

  _test_size(0);
  _test_height(-1);
  _test_del("key1", -1);
  _test_get("key1", NULL);
  _test_getkeys(0, NULL);

  // put nodes

  struct entry_t* entry0 = entry_create(strdup("key0"), data_create2(0, NULL));
  struct entry_t* entry1 = _entry_t_create(1);
  struct entry_t* entry2 = _entry_t_create(2);
  struct entry_t* entry3 = _entry_t_create(3);
  struct entry_t* entry4 = _entry_t_create(4);
  struct entry_t* entry5 = _entry_t_create(5);
  struct entry_t* entry6 = _entry_t_create(6);
  struct entry_t* entry8 = _entry_t_create(8);

  _test_put_entry(entry4);
  _test_put_entry(entry3);
  _test_put_entry(entry8);
  _test_put_entry(entry5);
  _test_put_entry(entry2);
  _test_put_entry(entry0);
  _test_put_entry(entry6);
  _test_put_entry(entry1);

  entry_destroy(entry0);
  entry_destroy(entry1);
  entry_destroy(entry2);
  entry_destroy(entry3);
  entry_destroy(entry4);
  entry_destroy(entry5);
  entry_destroy(entry6);
  entry_destroy(entry8);

  // query tree with nodes

  _test_size(8);
  _test_height(4);
  char* all_keys[8] = {"key0", "key1", "key2", "key3", "key4", "key5", "key6", "key8"};
  _test_getkeys(8, all_keys);

  _test_del("key2", +1);
  _test_size(7);
  _test_height(3);
  _test_del("key5", +1);
  _test_size(6);
  _test_height(3);
  char* remaining_keys[6] = {"key0", "key1", "key3", "key4", "key6", "key8"};
  _test_getkeys(6, remaining_keys);
  _test_get("key0", NULL);
  _test_get("key1", "value1");
  _test_get("key2", NULL);
  _test_get("key3", "value3");
  _test_get("key4", "value4");
  _test_get("key6", "value6");
  _test_get("key8", "value8");

  tree_skel_destroy();
  printTestDone();
}

// **************************************************************

int main() {
  test__tasks();

  test__tree_skel__invoke__tree_not_initialized();
  test__tree_skel__invoke__NULL_message_t();
  test__tree_skel__invoke__NULL_Message();
  test__tree_skel__invoke__unknown_message();
  test__tree_skel__invoke();

  printNoAssertionsFailed();
  return 0;
}