/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "logger-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tasks-private.h"

#include <pthread.h>
#include <stdlib.h>

#define SIZE_OF_TASK sizeof(struct task_t)
#define SIZE_OF_TASK_NODE sizeof(struct task_queue_node_t)
#define SIZE_OF_RESULT_NODE sizeof(struct task_result_list_node_t)

/* A node in a queue of tasks. */
struct task_queue_node_t {
  /* The task_t of this node. It must NOT be NULL. */
  struct task_t* task;
  /* The node containing the next task to invoke. It may be NULL. */
  struct task_queue_node_t* next;
};

/* A node in a linked-list of task-results. */
struct task_result_list_node_t {
  /* A unique identifier of the task. */
  int task_id;
  /* The task's result. */
  enum TaskResult task_result;

  /* The node containing the result of the previous task. It may be NULL. */
  struct task_result_list_node_t* previous;
};

/* The last task_id that was assigned to a task_t. */
int last_task_id_assigned;

/* The head of the queue of tasks.
 *  The head of this queue contains the oldest task that was queued and, when progressing to "next"
 *  nodes, task_ids are increasingly higher.
 */
struct task_queue_node_t* tasks_queue_head;
pthread_mutex_t tasks_queue_lock;
pthread_cond_t tasks_queue_change;

/* The head of the linked-list of task-results.
 *  The head of this list contains the result with the greatest task_id and, when progressing to
 *  "previous" nodes, task_ids are decreasingly lower (results refer to older tasks).
 */
struct task_result_list_node_t* tasks_results_list_head;
pthread_mutex_t tasks_results_list_lock;
pthread_cond_t tasks_results_list_change;

int destroyed = 0;

int tasks_init() {
  last_task_id_assigned = -1;

  tasks_queue_head = NULL;
  if (pthread_mutex_init(&tasks_queue_lock, NULL) < 0) {
    logger_perror("tasks_init", "Failed to init tasks_queue_lock");
    return -1;
  }
  if (pthread_cond_init(&tasks_queue_change, NULL) < 0) {
    logger_perror("tasks_init", "Failed to init tasks_queue_change");
    return -1;
  }

  tasks_results_list_head = NULL;
  if (pthread_mutex_init(&tasks_results_list_lock, NULL) < 0) {
    logger_perror("tasks_init", "Failed to init tasks_results_list_lock");
    return -1;
  }
  if (pthread_cond_init(&tasks_results_list_change, NULL) < 0) {
    logger_perror("tasks_init", "Failed to init tasks_results_list_change");
    return -1;
  }
  return 0;
}

struct task_t* _task_create(Message* op_code_and_args) {
  struct task_t* task = malloc(SIZE_OF_TASK);
  if (task == NULL) {
    logger_error_malloc_failed("_task_create");
    return NULL;
  }
  task->op_code_and_args = Message_dup(op_code_and_args);
  if (task->op_code_and_args == NULL) {
    logger_error_malloc_failed("_task_create");
    free(task);
    return NULL;
  }
  task->task_id = (++last_task_id_assigned);
  return task;
}

int tasks_add_task(Message* op_code_and_args) {
  if (op_code_and_args == NULL) {
    logger_error_invalid_arg("Queue_add_task", "op_code_and_args", "NULL");
    return -1;
  }
  struct task_t* task = _task_create(op_code_and_args);
  if (task == NULL) {
    return -1;
  }

  struct task_queue_node_t* node = malloc(SIZE_OF_TASK_NODE);
  if (node == NULL) {
    logger_error_malloc_failed("queue_add_task");
    return -1;
  }
  node->task = task;
  node->next = NULL;

  pthread_mutex_lock(&tasks_queue_lock);
  logger_debug("Queueing task with task_id=%d ...\n", task->task_id);

  if (tasks_queue_head == NULL) {
    tasks_queue_head = node;
  } else {
    struct task_queue_node_t* last_stored_task = tasks_queue_head;
    while (last_stored_task->next != NULL) {
      last_stored_task = last_stored_task->next;
    }
    last_stored_task->next = node;
  }

  pthread_cond_broadcast(&tasks_queue_change);
  pthread_mutex_unlock(&tasks_queue_lock);
  return task->task_id;
}

struct task_t* tasks_get_next() {
  pthread_mutex_lock(&tasks_queue_lock);
  while (destroyed == 0 && tasks_queue_head == NULL) {
    logger_debug("Waiting for next task...\n");
    pthread_cond_wait(&tasks_queue_change, &tasks_queue_lock);
  }
  if (destroyed) {
    return NULL;
  }
  struct task_t* task = tasks_queue_head->task;
  pthread_mutex_unlock(&tasks_queue_lock);
  return task;
}

void _task_destroy(struct task_t* task) {
  if (task != NULL) {
    Message_destroy(task->op_code_and_args);
    free(task);
  }
}

int tasks_set_result(int task_id, enum TaskResult task_result) {
  pthread_mutex_lock(&tasks_queue_lock);
  pthread_mutex_lock(&tasks_results_list_lock);
  logger_debug("Setting the result of the task with task_id=%d...\n", task_id);

  if (tasks_queue_head == NULL || tasks_queue_head->task->task_id != task_id) {
    logger_error_invalid_argi("task_set_result", "task_id", task_id);
    return -1;
  }
  if (task_result == NOT_EXECUTED) {
    logger_error_invalid_arg("task_set_result", "task_result", "NOT_EXECUTED");
    return -1;
  }

  struct task_result_list_node_t* result_node = malloc(SIZE_OF_RESULT_NODE);
  if (result_node == NULL) {
    logger_error_malloc_failed("task_set_result");
    return -1;
  }
  result_node->task_id = task_id;
  result_node->task_result = task_result;
  result_node->previous = tasks_results_list_head;
  tasks_results_list_head = result_node;

  struct task_queue_node_t* task_node = tasks_queue_head;
  tasks_queue_head = task_node->next;
  _task_destroy(task_node->task);
  task_node->next = NULL;
  free(task_node);

  pthread_mutex_unlock(&tasks_results_list_lock);
  pthread_mutex_unlock(&tasks_queue_lock);
  return 0;
}

enum TaskResult tasks_get_result(int task_id) {
  pthread_mutex_lock(&tasks_results_list_lock);
  logger_debug("Getting the result of the task with task_id=%d...\n", task_id);

  struct task_result_list_node_t* current_node = tasks_results_list_head;
  while (current_node != NULL && current_node->task_id >= task_id) {
    if (current_node->task_id == task_id) {
      enum TaskResult task_result = current_node->task_result;
      pthread_mutex_unlock(&tasks_results_list_lock);
      return task_result;
    }
    current_node = current_node->previous;
  }

  pthread_mutex_unlock(&tasks_results_list_lock);
  return NOT_EXECUTED;
}

/* Recursively destroys all nodes of the queue starting at the given node. */
void _tasks_queue_destroy(struct task_queue_node_t* node) {
  if (node != NULL) {
    _task_destroy(node->task);
    _tasks_queue_destroy(node->next);
    free(node);
  }
}

/* Recursively destroys all nodes of the linked-list starting at the given node. */
void _tasks_result_list_destroy(struct task_result_list_node_t* node) {
  if (node != NULL) {
    _tasks_result_list_destroy(node->previous);
    free(node);
  }
}

void tasks_destroy() {
  last_task_id_assigned = -1;
  destroyed = 1;

  _tasks_queue_destroy(tasks_queue_head);
  tasks_queue_head = NULL;
  pthread_mutex_destroy(&tasks_queue_lock);
  pthread_cond_broadcast(&tasks_queue_change);
  pthread_cond_destroy(&tasks_queue_change);

  _tasks_result_list_destroy(tasks_results_list_head);
  tasks_results_list_head = NULL;
  pthread_mutex_destroy(&tasks_results_list_lock);
  pthread_cond_destroy(&tasks_results_list_change);
}