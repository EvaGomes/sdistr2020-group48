/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "logger-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tasks-private.h"

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

/* The head of the linked-list of task-results.
 *  The head of this list contains the result with the greatest task_id and, when progressing to
 *  "previous" nodes, task_ids are decreasingly lower (results refer to older tasks).
 */
struct task_result_list_node_t* tasks_results_list_head;

int tasks_init() {
  last_task_id_assigned = -1;
  tasks_queue_head = NULL;
  tasks_results_list_head = NULL;
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

  if (tasks_queue_head == NULL) {
    tasks_queue_head = node;
  } else {
    struct task_queue_node_t* last_stored_task = tasks_queue_head;
    while (last_stored_task->next != NULL) {
      last_stored_task = last_stored_task->next;
    }
    last_stored_task->next = node;
  }

  return 0;
}

/* Returns the next task_t to be executed. The task is not considered done until its result gets set
 * using function task_set_result.
 */
struct task_t* tasks_get_next() {
  return tasks_queue_head == NULL ? NULL : tasks_queue_head->task;
}

void _task_destroy(struct task_t* task) {
  if (task != NULL) {
    Message_destroy(task->op_code_and_args);
    free(task);
  }
}

int tasks_set_result(int task_id, enum TaskResult task_result) {
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
  return 0;
}

enum TaskResult tasks_get_result(int task_id) {
  struct task_result_list_node_t* current_node = tasks_results_list_head;
  while (current_node != NULL && current_node->task_id >= task_id) {
    if (current_node->task_id == task_id) {
      return current_node->task_result;
    }
    current_node = current_node->previous;
  }
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
  _tasks_queue_destroy(tasks_queue_head);
  tasks_queue_head = NULL;
  _tasks_result_list_destroy(tasks_results_list_head);
  tasks_results_list_head = NULL;
}