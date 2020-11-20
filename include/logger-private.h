/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#ifndef _LOGGER_PRIVATE_H
#define _LOGGER_PRIVATE_H

void logger_debug(const char* msg_format, ...);
void logger_debug__sockfd(int sockfd, const char* msg);

void logger_info(const char* msg_format, ...);
void logger_info__sockfd(int sockfd, const char* msg);

void logger_error(const char* location, const char* msg_format, ...);
void logger_error_invalid_arg(const char* location, const char* arg_name, const char* arg_value);
void logger_error_invalid_args(const char* location);
void logger_error_malloc_failed(const char* location);
void logger_error__sockfd(int sockfd, const char* location, const char* msg);
void logger_perror(const char* location, const char* msg_format, ...);
void logger_perror__sockfd(int sockfd, const char* location, const char* msg);

#endif