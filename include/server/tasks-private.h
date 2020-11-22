/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */
#ifndef _TASK_PRIVATE_H
#define _TASK_PRIVATE_H

enum TaskResult {
  SUCCESSFUL = 1,
  NOT_EXECUTED = 0,
  FAILED = -1
};

/* Initializes the structures to store tasks and results.
 *  Returns 0 if successful, or -1 if an error occurred.
 */
int tasks_init();

/* Generates a unique identifier for a task. */
int tasks_generate_id();

/* Sets the result of a task that has just been invoked.
 *  Returns 0 if the result was set successfully, or -1 if an error occurred.
 */
int tasks_set_result(int task_id, enum TaskResult task_result);

/* Returns the result of the task with the given identifier, or NULL if an error occurred. */
enum TaskResult tasks_get_result(int task_id);

/* Destroys the stored tasks and results. */
void tasks_destroy();

#endif
