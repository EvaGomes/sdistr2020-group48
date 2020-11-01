/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tree.h"
#include <stdio.h>

struct tree_t* tree;

int tree_skel_init() {
  tree = tree_create();
  return tree == NULL ? -1 : 0;
}

void tree_skel_destroy() {
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
  char* key = msg_to_string(request->key);
  int del_result = tree_del(tree, key);
  response->op_code = (del_result == 0) ? (request->op_code + 1) : OP_ERROR;
  response->content_case = CT_NONE;
}

void _invoke_tree_get(Message* request, Message* response) {
  char* key = msg_to_string(request->key);
  struct data_t* value = tree_get(tree, key);
  if (value != NULL) {
    response->op_code = request->op_code + 1;
    response->content_case = CT_VALUE;
    response->value = data_to_msg(value);
  } else {
    response->op_code = OP_ERROR;
    response->content_case = CT_NONE;
  }
}

void _invoke_tree_put(Message* request, struct entry_t* entry, Message* response) {
  int put_result = tree_put(tree, entry->key, entry->value);
  response->op_code = (put_result == 0) ? (request->op_code + 1) : OP_ERROR;
  response->content_case = CT_NONE;
}

void _invoke_tree_get_keys(Message* request, Message* response) {
  char** keys = tree_get_keys(tree);
  if (keys != NULL) {
    response->op_code = request->op_code + 1;
    response->content_case = CT_KEYS;
    response->keys = keys_to_msg(keys);
  } else {
    response->op_code = OP_ERROR;
    response->content_case = CT_NONE;
  }
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
      fprintf(stderr, "\nERR: tree_skel#invoke: could not convert EntryMessage to entry_t\n");
    } else {
      _invoke_tree_put(request, entry, response);
    }
  }

  else if (request->op_code == OP_GETKEYS) {
    _invoke_tree_get_keys(request, response);
  }
}

int invoke(struct message_t* message) {
  if (tree == NULL) {
    fprintf(stderr, "\nERR: tree_skel#invoke: tree_skel_init was not invoked\n");
    return -1;
  } else if (message == NULL || message->msg == NULL) {
    fprintf(stderr, "\nERR: tree_skel#invoke: received msg is NULL\n");
    return -1;
  }

  Message* request = message->msg;

  Message* response;
  message__init(response);
  response->op_code = OP_BAD;
  message->msg = response;

  _invoke(request, response);

  message__free_unpacked(request, NULL);

  if (response->op_code != OP_BAD) {
    // correctly field by one of the operations
    return 0;
  } else {
    response->op_code = OP_ERROR;
    response->content_case = CT_INT_RESULT;
    response->int_result = -1;
    fprintf(stderr, "\nERR: tree_skel#invoke: unknown msg: op_code=%d, content_case=%d\n",
            request->op_code, request->content_case);
    return -1;
  }
}
