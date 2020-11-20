/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_DEBUG_MESSAGES 0
#define LOG_INFO_MESSAGES 1
#define LOG_ERROR_MESSAGES 1

#if LOG_DEBUG_MESSAGES /*************************************************************************/

void logger_debug(const char* msg_format, ...) {
  if (LOG_DEBUG_MESSAGES) {
    va_list args;
    va_start(args, msg_format);
    vprintf(msg_format, args);
    va_end(args);
  }
}

void logger_debug__sockfd(int sockfd, const char* msg) {
  if (LOG_DEBUG_MESSAGES) {
    printf("sockfd=%d - %s\n", sockfd, msg);
  }
}

#else
void logger_debug(const char* msg_format, ...) {}
void logger_debug__sockfd(int sockfd, const char* msg) {}
#endif

#if LOG_INFO_MESSAGES /**************************************************************************/

void logger_info(const char* msg_format, ...) {
  va_list args;
  va_start(args, msg_format);
  vprintf(msg_format, args);
  va_end(args);
}

void logger_info__sockfd(int sockfd, const char* msg) {
  printf("sockfd=%d - %s\n", sockfd, msg);
}

#else
void logger_info(const char* msg_format, ...) {}
void logger_info__sockfd(int sockfd, const char* msg) {}
#endif

#if LOG_ERROR_MESSAGES /**************************************************************************/

void logger_error(const char* location, const char* msg_format, ...) {
  fprintf(stderr, "ERROR at %s: ", location);

  va_list args;
  va_start(args, msg_format);
  vfprintf(stderr, msg_format, args);
  va_end(args);

  fprintf(stderr, "\n");
}

void logger_error_invalid_arg(const char* location, const char* arg_name, const char* arg_value) {
  fprintf(stderr, "ERROR at %s: Invalid arg (%s is %s)\n", location, arg_name, arg_value);
}

void logger_error_invalid_args(const char* location) {
  fprintf(stderr, "ERROR at %s: Invalid args\n", location);
}

void logger_error_malloc_failed(const char* location) {
  fprintf(stderr, "ERROR at %s: Failed to malloc\n", location);
}

void logger_error__sockfd(int sockfd, const char* location, const char* msg) {
  fprintf(stderr, "sockfd=%d - ERROR at %s: %s\n", sockfd, location, msg);
}

void logger_perror(const char* location, const char* msg_format, ...) {
  fprintf(stderr, "ERROR at %s: ", location);

  va_list args;
  va_start(args, msg_format);
  vfprintf(stderr, msg_format, args);
  va_end(args);

  fprintf(stderr, " (%s)\n", strerror(errno));
}

void logger_perror__sockfd(int sockfd, const char* location, const char* msg) {
  fprintf(stderr, "sockfd=%d - ERROR at %s: %s (%s)\n", sockfd, location, msg, strerror(errno));
}

#else
void logger_error(const char* location, const char* msg_format, ...) {}
void logger_error_invalid_arg(const char* location, const char* arg_name, const char* arg_value) {}
void logger_error_invalid_args(const char* location) {}
void logger_error_malloc_failed(const char* location) {}
void logger_perror(const char* location, const char* msg_format, ...) {}
void logger_error__sockfd(int sockfd, const char* location, const char* msg) {}
void logger_perror__sockfd(int sockfd, const char* location, const char* msg) {}
#endif