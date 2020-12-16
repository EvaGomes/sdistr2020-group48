/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "logger-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tasks-private.h"
#include "tree.h"
#include "zk-private.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct tree_t* tree;
pthread_mutex_t tree_lock;

pthread_t tasks_processor_thread;

/******** QUERIES (READ OPERATIONS) **************************************************************/

static void invoke_tree_size(Message* request, Message* response) {
  int size = tree_size(tree);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = size;
}

static void invoke_tree_height(Message* request, Message* response) {
  int height = tree_height(tree);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = height;
}

static void invoke_tree_get(Message* request, Message* response) {
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

static void invoke_tree_get_keys(Message* request, Message* response) {
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

static void invoke_write_operation(Message* request, Message* response) {
  int task_id = tasks_add_task(request);
  response->op_code = request->op_code + 1;
  response->content_case = CT_OP_ID;
  response->op_id = task_id;
}

static void invoke_verify(Message* request, Message* response) {
  enum TaskResult task_result = tasks_get_result(request->op_id);
  response->op_code = request->op_code + 1;
  response->content_case = CT_INT_RESULT;
  response->int_result = task_result;
}

static void invoke2(Message* request, Message* response) {
  pthread_mutex_lock(&tree_lock);
  if (request->op_code == OP_SIZE) {
    invoke_tree_size(request, response);
  } else if (request->op_code == OP_HEIGHT) {
    invoke_tree_height(request, response);
  } else if (request->op_code == OP_DEL && request->content_case == CT_KEY) {
    invoke_write_operation(request, response);
  } else if (request->op_code == OP_GET && request->content_case == CT_KEY) {
    invoke_tree_get(request, response);
  } else if (request->op_code == OP_PUT && request->content_case == CT_ENTRY) {
    invoke_write_operation(request, response);
  } else if (request->op_code == OP_GETKEYS) {
    invoke_tree_get_keys(request, response);
  } else if (request->op_code == OP_VERIFY && request->content_case == CT_OP_ID) {
    invoke_verify(request, response);
  }
  pthread_mutex_unlock(&tree_lock);
}

/******** WRITE OPERATIONS ***********************************************************************/

static int replicate_to_backup_server(struct task_t* task) {
  if (zk_get_tree_server_role() == BACKUP) {
    return 0; // this is the backup server already
  }

  char* backup_server = zk_get_backup_tree_server();
  if (backup_server == NULL) {
    return -1;
  }
  char* ip_address;
  int port;
  int parse_result = parse_address_port(backup_server, &ip_address, &port);
  free(backup_server);
  if (parse_result < -1) {
    return -1;
  }

  int sockfd = server_connect(ip_address, port);
  free(ip_address);
  if (sockfd < -1) {
    return -1;
  }

  struct message_t* request = message_create();
  if (request == NULL) {
    close(sockfd);
    return -1;
  }
  request->msg = Message_dup(task->op_code_and_args);
  if (request->msg == NULL) {
    close(sockfd);
    free(request);
    return -1;
  }

  int send_result = network_send_message(sockfd, request);
  message_destroy(request);
  if (send_result < 0) {
    close(sockfd);
    return -1;
  }

  logger_debug("Replicated request to backup server...\n");
  close(sockfd);
  return 0;
}

static void process_tree_del(struct task_t* task) {
  if (tree_del(tree, task->op_code_and_args->key) < 0) {
    logger_error("process_tree_del", "Failed to delete, killing server");
    exit(1);
  }
  if (replicate_to_backup_server(task) < 0) {
    logger_error("process_tree_del", "Failed to replicate op to backup server, killing server");
    exit(1);
  }
  tasks_set_result(task->task_id, SUCCESSFUL);
}

static void process_tree_put(struct task_t* task) {
  struct entry_t* entry = msg_to_entry(task->op_code_and_args->entry);
  if ((entry == NULL) || (tree_put(tree, entry->key, entry->value) < 0)) {
    logger_error("process_tree_put", "Failed to put, killing server");
    exit(1);
  }
  entry_destroy(entry);
  if (replicate_to_backup_server(task) < 0) {
    logger_error("process_tree_put", "Failed to replicate op to backup server, killing server");
    exit(1);
  }
  tasks_set_result(task->task_id, SUCCESSFUL);
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
      process_tree_del(task);
    }

    else if (op_code == OP_PUT && content_case == CT_ENTRY) {
      process_tree_put(task);
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

  invoke2(request, response);
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
