/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "logger-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tasks-private.h"
#include "tree.h"

struct tree_t* tree;

int tree_skel_init() {
  tree = tree_create();
  int tasks_init_result = tasks_init();
  return (tree == NULL || tasks_init_result < 0) ? -1 : 0;
}

void tree_skel_destroy() {
  tasks_destroy();
  tree_destroy(tree);
}

void _invoke_tree_size(Message* request, Message* response) {
  int size = tree_size(tree);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = size;
}

void _invoke_tree_height(Message* request, Message* response) {
  int height = tree_height(tree);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = height;
}

void _invoke_tree_del(Message* request, Message* response) {
  int task_id = tasks_generate_id();
  response->op_code = request->op_code + 1;
  response->content_case = CT_OP_ID;
  response->op_id = task_id;

  int del_result = tree_del(tree, request->key);
  enum TaskResult task_result = del_result < 0 ? FAILED : SUCCESSFUL;
  tasks_set_result(task_id, task_result);
}

void _invoke_tree_get(Message* request, Message* response) {
  struct data_t* value = tree_get(tree, request->key);
  if (value != NULL) {
    response->op_code = request->op_code + 1;
    response->content_case = CT_VALUE;
    response->value = data_to_msg(value);
    data_destroy(value);
  } else {
    response->op_code = OP_ERROR;
    response->content_case = CT_NONE;
  }
}

void _invoke_tree_put(Message* request, struct entry_t* entry, Message* response) {
  int task_id = tasks_generate_id();
  response->op_code = request->op_code + 1;
  response->content_case = CT_OP_ID;
  response->op_id = task_id;

  int put_result = tree_put(tree, entry->key, entry->value);
  enum TaskResult task_result = put_result < 0 ? FAILED : SUCCESSFUL;
  tasks_set_result(task_id, task_result);
}

void _invoke_tree_get_keys(Message* request, Message* response) {
  char** keys = tree_get_keys(tree);
  if (keys != NULL) {
    response->op_code = request->op_code + 1;
    response->content_case = CT_KEYS;
    response->keys = keys_to_msg(keys);
    tree_free_keys(keys);
  } else {
    response->op_code = OP_ERROR;
    response->content_case = CT_NONE;
  }
}

void _invoke_verify(Message* request, Message* response) {
  enum TaskResult task_result = tasks_get_result(request->op_id);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = task_result;
}

void _invoke(Message* request, Message* response) {

  if (request->op_code == OP_SIZE) {
    _invoke_tree_size(request, response);
  }

  else if (request->op_code == OP_HEIGHT) {
    _invoke_tree_height(request, response);
  }

  else if (request->op_code == OP_DEL && request->content_case == CT_KEY) {
    _invoke_tree_del(request, response);
  }

  else if (request->op_code == OP_GET && request->content_case == CT_KEY) {
    _invoke_tree_get(request, response);
  }

  else if (request->op_code == OP_PUT && request->content_case == CT_ENTRY) {
    struct entry_t* entry = msg_to_entry(request->entry);
    if (entry == NULL) {
      logger_error("tree_skel#_invoke", "Failed to convert EntryMessage to entry_t");
    } else {
      _invoke_tree_put(request, entry, response);
      entry_destroy(entry);
    }
  }

  else if (request->op_code == OP_GETKEYS) {
    _invoke_tree_get_keys(request, response);
  }

  else if (request->op_code == OP_VERIFY && request->content_case == CT_OP_ID) {
    _invoke_verify(request, response);
  }
}

int invoke(struct message_t* message) {
  if (tree == NULL) {
    logger_error("tree_skel#invoke", "tree_skel_init was not invoked");
    return -1;
  } else if (message == NULL || message->msg == NULL) {
    logger_error_invalid_arg("tree_skel#invoke", "message", "NULL");
    return -1;
  }

  Message* request = message->msg;

  Message* response = Message_create();
  response->op_code = OP_BAD;

  _invoke(request, response);
  if (response->op_code == OP_BAD) {
    // none of the operations filled the response correctly
    response->op_code = OP_ERROR;
    response->content_case = CT_INT_RESULT;
    response->int_result = -1;
    logger_error("tree_skel#invoke", "Cannot handle op_code=%d, content_case=%d", request->op_code,
                 request->content_case);
  }

  message->msg = response;
  message__free_unpacked(request, NULL);
  return 0;
}
