/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "logger-private.h"
#include "serialization-private.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE_OF_INT sizeof(int)

struct in_addr* get_host() {
  char hostname[256];
  gethostname(hostname, 256);

  struct hostent* host = gethostbyname(hostname);
  if (host->h_addrtype != AF_INET) {
    logger_error("get_host", "Host has unexpected h_addrtype %d (expected: %d)", host->h_addrtype,
                 AF_INET);
    return NULL;
  }
  return (struct in_addr*) host->h_addr;
}

/* Reads buffer_size bytes from the socket with the given descriptor (by chunks if needed).
 *  Expected sockfd and buffer_size to be positive numbers.
 *  Returns the read buffer, or NULL if the connection closed or an error occurred.
 */
void* _network_read_buffer(int sockfd, int buffer_size) {
  void* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    logger_error_malloc_failed("_network_read_buffer");
    return NULL;
  }

  int read_size = 0;
  while (read_size < buffer_size) {
    int read_chunk_size = read(sockfd, buffer + read_size, buffer_size - read_size);
    logger_debug__sockfd(sockfd, "Reading data...");
    if (read_chunk_size < 0) {
      if (errno == EINTR) {
        continue; // retry
      } else {
        logger_perror__sockfd(sockfd, "_network_read_buffer", "Failed to read data");
        free(buffer);
        return NULL;
      }
    } else if (read_chunk_size == 0) {
      logger_error__sockfd(sockfd, "_network_read_buffer",
                           "Failed to read data (Connection closed)");
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
    logger_error__sockfd(sockfd, "network_receive_message", "Failed to read data (Invalid size)");
    return NULL;
  }
  logger_debug__sockfd(sockfd, "Received size of data");

  char* read_message_buffer = _network_read_buffer(sockfd, message_size);
  if (read_message_buffer == NULL) {
    return NULL;
  }
  logger_debug__sockfd(sockfd, "Received data");

  struct message_t* message = buffer_to_message(read_message_buffer, message_size);
  free(read_message_buffer);
  if (message == NULL) {
    logger_error__sockfd(sockfd, "network_receive_message",
                         "Failed to convert buffer to message_t");
    return NULL;
  }
  logger_debug__sockfd(sockfd, "Converted data to message_t");

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
    logger_debug__sockfd(sockfd, "Sending data...");
    if (written_chunk_size < 0) {
      if (errno == EINTR) {
        continue; // retry
      } else {
        logger_perror__sockfd(sockfd, "_network_send_buffer", "Failed to send data");
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
    logger_error__sockfd(sockfd, "network_send_message", "Failed to convert message_t to buffer");
    return -1;
  }
  logger_debug__sockfd(sockfd, "Converted message_t to data");

  int converted_buffer_size = htonl(buffer_size);
  if (_network_send_buffer(sockfd, &converted_buffer_size, SIZE_OF_INT) < 0) {
    return -1;
  }
  logger_debug__sockfd(sockfd, "Sent size of data");

  if (_network_send_buffer(sockfd, (void*) buffer, buffer_size) != 0) {
    free(buffer);
    return -1;
  }
  logger_debug__sockfd(sockfd, "Sent data");

  free(buffer);
  return 0;
}
