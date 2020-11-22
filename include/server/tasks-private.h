/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */
#ifndef _TASK_PRIVATE_H
#define _TASK_PRIVATE_H

struct task_t {
  /* A unique identifier of this task. */
  int task_id;
  /* The description of the task to perform (the code of the write-operation and the arguments with
   * which to invoke it). It must NOT be NULL.
   */
  Message* op_code_and_args;
};

enum TaskResult {
  SUCCESSFUL = 1,
  NOT_EXECUTED = 0,
  FAILED = -1
};

/* Initializes the structures to store tasks and results.
 *  Returns 0 if successful, or -1 if an error occurred.
 */
int tasks_init();

/* Adds a new task to the queue of tasks.
 *  Returns 0 if the task was successfully queued, or -1 if an error occurred.
 */
int tasks_add_task(Message* op_code_and_args);

/* Returns the next task_t to be executed. The task is not considered done until its result gets set
 * using function task_set_result.
 */
struct task_t* tasks_get_next();

/* Sets the result of a task that has just been invoked.
 *  Returns 0 if the result was set successfully, or -1 if an error occurred.
 */
int tasks_set_result(int task_id, enum TaskResult task_result);

/* Returns the result of the task with the given identifier, or NULL if an error occurred. */
enum TaskResult tasks_get_result(int task_id);

/* Destroys the stored tasks and results. */
void tasks_destroy();

#endif
