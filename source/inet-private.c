/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "serialization-private.h"

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE_OF_INT sizeof(int)

int ignore_SIGPIPE_signals() {
  struct sigaction s = {};
  s.sa_handler = SIG_IGN;

  int result = sigaction(SIGPIPE, &s, NULL);
  if (result < 0) {
    fprintf(stderr, "Failed to register handler to ignore SIGPIPE signals\n");
  }
  return result;
}

/* Reads buffer_size bytes from the socket with the given descriptor (by chunks if needed).
 *  Expected sockfd and buffer_size to be positive numbers.
 *  Returns the read buffer, or NULL if the connection closed or an error occurred.
 */
void* _network_read_buffer(int sockfd, int buffer_size) {
  void* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "\nERR: _network_read_buffer: malloc failed\n");
    return NULL;
  }

  int read_size = 0;
  while (read_size < buffer_size) {
    int read_chunk_size = read(sockfd, buffer + read_size, buffer_size - read_size);
    printf("sockfd=%d - Reading data...\n", sockfd);
    if (read_chunk_size < 0) {
      if (errno == EINTR) {
        continue; // retry
      } else {
        fprintf(stderr, "sockfd=%d - Error while reading data (%s)\n", sockfd, strerror(errno));
        free(buffer);
        return NULL;
      }
    } else if (read_chunk_size == 0) {
      fprintf(stderr, "sockfd=%d - Error while reading data (Connection closed)\n", sockfd);
      free(buffer);
      return NULL;
    } else {
      read_size += read_chunk_size;
    }
  }
  return buffer;
}

struct message_t* network_receive_message(int sockfd) {
  int* read_message_size = _network_read_buffer(sockfd, SIZE_OF_INT);
  if (read_message_size == NULL) {
    return NULL;
  }
  int message_size = ntohl(*read_message_size);
  free(read_message_size);
  if (message_size < 0) {
    fprintf(stderr, "sockfd=%d - Error while reading data (invalid size)\n", sockfd);
    return NULL;
  }
  printf("sockfd=%d - Received size of data\n", sockfd);

  char* read_message_buffer = _network_read_buffer(sockfd, message_size);
  if (read_message_buffer == NULL) {
    return NULL;
  }
  printf("sockfd=%d - Received data\n", sockfd);

  struct message_t* message = buffer_to_message(read_message_buffer, message_size);
  free(read_message_buffer);
  if (message == NULL) {
    fprintf(stderr, "sockfd=%d - Error while converting buffer to message_t\n", sockfd);
    return NULL;
  }
  printf("sockfd=%d - Converted data to message_t\n", sockfd);

  return message;
}

/* Sends the given buffer to the socket with the given descriptor (by chunks if needed).
 *  Expects sockfd and buffer_size to be positive numbers, and buffer to not be NULL.
 *  Returns 0 if the whole buffer was successfully sent, or a negative number otherwise.
 */
int _network_send_buffer(int sockfd, void* buffer, int buffer_size) {
  int written_size = 0;
  while (written_size < buffer_size) {
    int written_chunk_size = write(sockfd, buffer + written_size, buffer_size - written_size);
    printf("sockfd=%d - Sending data...\n", sockfd);
    if (written_chunk_size < 0) {
      if (errno == EINTR) {
        continue; // retry
      } else {
        fprintf(stderr, "sockfd=%d - Error while sending data (%s)\n", sockfd, strerror(errno));
        return -1;
      }
    } else {
      written_size += written_chunk_size;
    }
  }
  return 0;
}

int network_send_message(int sockfd, struct message_t* message) {
  char* buffer;
  int buffer_size = message_to_buffer(message, &buffer);
  if (buffer_size < 0) {
    fprintf(stderr, "sockfd=%d - Error while converting message_t to buffer\n", sockfd);
    return -1;
  }
  printf("sockfd=%d - Converted message_t to data\n", sockfd);

  int converted_buffer_size = htonl(buffer_size);
  if (_network_send_buffer(sockfd, &converted_buffer_size, SIZE_OF_INT) < 0) {
    return -1;
  }
  printf("sockfd=%d - Sent size of data\n", sockfd);

  if (_network_send_buffer(sockfd, (void*) buffer, buffer_size) != 0) {
    free(buffer);
    return -1;
  }
  printf("sockfd=%d - Sent data\n", sockfd);

  free(buffer);
  return 0;
}
