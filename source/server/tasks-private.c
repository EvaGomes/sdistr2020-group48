/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "tasks-private.h"
#include "logger-private.h"
#include "sdmessage.pb-c.h"

#include <stdlib.h>

#define SIZE_OF_RESULT_NODE sizeof(struct task_result_list_node_t)

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

/* The head of the linked-list of task-results.
 *  The head of this list contains the result with the greatest task_id and, when progressing to
 *  "previous" nodes, task_ids are decreasingly lower (results refer to older tasks).
 */
struct task_result_list_node_t* tasks_results_list_head;

int tasks_init() {
  last_task_id_assigned = -1;
  tasks_results_list_head = NULL;
  return 0;
}

int tasks_generate_id() {
  last_task_id_assigned += 1;
  return last_task_id_assigned;
}

int tasks_set_result(int task_id, enum TaskResult task_result) {
  if (tasks_results_list_head != NULL && tasks_results_list_head->task_id >= task_id) {
    logger_error_invalid_argi("task_set_result", "task_id", task_id);
    return -1;
  }
  if (task_result == NOT_EXECUTED) {
    logger_error_invalid_arg("task_set_result", "task_result", "NOT_EXECUTED");
    return -1;
  }

  struct task_result_list_node_t* node = malloc(SIZE_OF_RESULT_NODE);
  if (node == NULL) {
    logger_error_malloc_failed("task_set_result");
    return -1;
  }
  node->task_id = task_id;
  node->task_result = task_result;
  node->previous = tasks_results_list_head;

  tasks_results_list_head = node;
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

/* Recursively destroys all nodes of the linked-list starting at the given node. */
void _tasks_result_list_destroy(struct task_result_list_node_t* node) {
  if (node != NULL) {
    _tasks_result_list_destroy(node->previous);
    free(node);
  }
}

void tasks_destroy() {
  last_task_id_assigned = -1;
  _tasks_result_list_destroy(tasks_results_list_head);
}