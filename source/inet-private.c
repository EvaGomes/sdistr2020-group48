/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "serialization-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int ignore_SIGPIPE_signals() {
  struct sigaction s = {};
  s.sa_handler = SIG_IGN;

  int result = sigaction(SIGPIPE, &s, NULL);
  if (result < 0) {
    fprintf(stderr, "Failed to register handler to ignore SIGPIPE signals\n");
  }
  return result;
}

struct message_t* network_receive_message(int sockfd) {
  char* buffer = malloc(MAX_MSG);
  if (buffer == NULL) {
    fprintf(stderr, "ERR: network_receive_message: malloc failed\n");
    return NULL;
  }

  int read_size = read(sockfd, (void*) buffer, MAX_MSG);
  if (read_size < 0) {
    fprintf(stderr, "sockfd=%d - Error while reading data\n", sockfd);
    free(buffer);
    return NULL;
  }
  printf("sockfd=%d - Received data\n", sockfd);

  struct message_t* message = buffer_to_message(buffer, read_size);

  free(buffer);

  if (message == NULL) {
    fprintf(stderr, "sockfd=%d - Error while converting buffer to message_t\n", sockfd);
    return NULL;
  }
  printf("sockfd=%d - Converted data to message_t: { op_code:%d, content_case:%d }\n", sockfd,
         message->msg->op_code, message->msg->content_case);
  return message;
}

int network_send_message(int sockfd, struct message_t* message) {
  char* buffer;
  int buffer_size = message_to_buffer(message, &buffer);
  if (buffer_size < 0) {
    fprintf(stderr, "sockfd=%d - Error while converting message_t to buffer\n", sockfd);
    return -1;
  }
  printf("sockfd=%d - Converted message_t { op_code:%d, content_case:%d } to buffer\n", sockfd,
         message->msg->op_code, message->msg->content_case);

  int written_size = write(sockfd, (void*) buffer, buffer_size);
  if (written_size != buffer_size) {
    fprintf(stderr, "sockfd=%d - Error while sending message\n", sockfd);
    return -1;
  }
  printf("sockfd=%d - Sent data\n", sockfd);

  return 0;
}
