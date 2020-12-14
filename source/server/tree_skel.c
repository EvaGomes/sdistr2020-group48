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

#include <pthread.h>

struct tree_t* tree;
pthread_mutex_t tree_lock;

pthread_t tasks_processor_thread;

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

void _invoke_write_operation(Message* request, Message* response) {
  int task_id = tasks_add_task(request);
  response->op_code = request->op_code + 1;
  response->content_case = CT_OP_ID;
  response->op_id = task_id;
}

void _invoke_verify(Message* request, Message* response) {
  enum TaskResult task_result = tasks_get_result(request->op_id);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = task_result;
}

void _invoke(Message* request, Message* response) {
  pthread_mutex_lock(&tree_lock);
  if (request->op_code == OP_SIZE) {
    _invoke_tree_size(request, response);
  } else if (request->op_code == OP_HEIGHT) {
    _invoke_tree_height(request, response);
  } else if (request->op_code == OP_DEL && request->content_case == CT_KEY) {
    _invoke_write_operation(request, response);
  } else if (request->op_code == OP_GET && request->content_case == CT_KEY) {
    _invoke_tree_get(request, response);
  } else if (request->op_code == OP_PUT && request->content_case == CT_ENTRY) {
    _invoke_write_operation(request, response);
  } else if (request->op_code == OP_GETKEYS) {
    _invoke_tree_get_keys(request, response);
  } else if (request->op_code == OP_VERIFY && request->content_case == CT_OP_ID) {
    _invoke_verify(request, response);
  }
  pthread_mutex_unlock(&tree_lock);
}

void _process_tree_del(int task_id, char* key) {
  int del_result = tree_del(tree, key);
  enum TaskResult task_result = del_result < 0 ? FAILED : SUCCESSFUL;
  tasks_set_result(task_id, task_result);
}

void _process_tree_put(int task_id, struct entry_t* entry) {
  enum TaskResult task_result;
  if (entry == NULL) {
    task_result = FAILED;
  } else {
    int put_result = tree_put(tree, entry->key, entry->value);
    task_result = put_result < 0 ? FAILED : SUCCESSFUL;
  }
  tasks_set_result(task_id, task_result);
}

void* process_tasks(void* params) {
  if (tree == NULL) {
    logger_error("tree_skel#process_tasks", "tree_skel_init was not invoked");
    return NULL;
  }

  while (1) {
    struct task_t* task = tasks_get_next(); // blocking function
    if (task == NULL) {
      return NULL;
    }

    int task_id = task->task_id;
    int op_code = task->op_code_and_args->op_code;
    int content_case = task->op_code_and_args->content_case;

    pthread_mutex_lock(&tree_lock);
    logger_debug("Processing task with task_id=%d ...\n", task_id);

    if (op_code == OP_DEL && content_case == CT_KEY) {
      _process_tree_del(task_id, task->op_code_and_args->key);
    }

    else if (op_code == OP_PUT && content_case == CT_ENTRY) {
      struct entry_t* entry = msg_to_entry(task->op_code_and_args->entry);
      _process_tree_put(task_id, entry);
      entry_destroy(entry);
    }

    else {
      logger_error("tree_skel#process_tasks", "Cannot handle op_code=%d, content_case=%d", op_code,
                   content_case);
      tasks_set_result(task_id, FAILED);
    }

    pthread_mutex_unlock(&tree_lock);
  }
}

/******** PUBLIC FUNCTIONS ***********************************************************************/

int tree_skel_init() {
  tree = tree_create();
  int tasks_init_result = tasks_init();
  int thread_creation_result = pthread_create(&tasks_processor_thread, NULL, process_tasks, NULL);
  if (tree == NULL || tasks_init_result < 0 || thread_creation_result < 0) {
    return -1;
  }
  if (pthread_mutex_init(&tree_lock, NULL) < 0) {
    logger_perror("tree_skel_init", "Failed to init tree_lock");
    return -1;
  }

  pthread_detach(tasks_processor_thread);
  return 0;
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
  Message_destroy(request);
  return 0;
}

void tree_skel_destroy() {
  tasks_destroy();
  pthread_mutex_destroy(&tree_lock);
  tree_destroy(tree);
}
