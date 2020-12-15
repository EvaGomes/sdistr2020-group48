/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "logger-private.h"
#include "message-private.h"
#include "tree_skel.h"

#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SIZE_OF_SOCKADDR sizeof(struct sockaddr)

/* The number of clients supported simultaneously by the server. */
#define MAX_CLIENT_SOCKETS 5

int network_server_init(short port) {
  int listening_socket;

  if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    logger_perror("network_server_init", "Failed to create TCP socket");
    return -1;
  }

  int enable = 1;
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    logger_perror("network_server_init", "Failed to allow reuse of local addresses");
  }


  struct in_addr* ip_address = get_host();
  if (ip_address == NULL) {
    logger_perror("network_server_init", "Failed to get host");
    close(listening_socket);
    return -1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = port;
  server.sin_addr = *ip_address;

  if (bind(listening_socket, (struct sockaddr*) &server, sizeof(server)) < 0) {
    logger_perror("network_server_init", "Failed to bind the socket to the local address");
    close(listening_socket);
    return -1;
  }

  if (listen(listening_socket, 0) < 0) {
    logger_perror("network_server_init", "Failed to prepare socket for listening");
    close(listening_socket);
    return -1;
  }

  if (tree_skel_init() < 0) {
    logger_perror("network_server_init", "Failed to init the binary tree");
    close(listening_socket);
    return -1;
  }

  logger_info("\nServer is ready...\n");
  return listening_socket;
}

struct message_t* network_receive(int client_socket) {
  return network_receive_message(client_socket);
}

int network_send(int client_socket, struct message_t* msg) {
  return network_send_message(client_socket, msg);
}

/* Closes all client sockets (does not close the listening_socket). */
void _close_all_client_sockets(struct pollfd* sockets) {
  for (int i = 1; i <= MAX_CLIENT_SOCKETS; i++) {
    if (sockets[i].fd != -1) {
      close(sockets[i].fd);
    }
  }
}

/* Handles the events of the listening socket.
 *  Expects the first position of the given array of sockets to be the listening_socket.
 */
void _handle_listening_socket_events(struct pollfd* sockets, int* client_sockets_count) {
  struct pollfd listening_socket = sockets[0];
  if ((listening_socket.revents & POLLIN) && (*client_sockets_count < MAX_CLIENT_SOCKETS)) {
    int i = *client_sockets_count + 1;
    // try to accept new connection and set it at index i
    struct sockaddr client;
    socklen_t size_client = SIZE_OF_SOCKADDR;
    sockets[i].fd = accept(listening_socket.fd, &client, &size_client);
    if (sockets[i].fd > 0) {
      sockets[i].events = POLLIN;
      sockets[i].revents = 0;
      *client_sockets_count += 1;
      logger_info__sockfd(sockets[i].fd, "Accepted client connection");
    } else {
      logger_error("_handle_listening_socket_events", "Failed to accept client connection");
    }
  }
}

/* Reads the message from the given client socket and sends the response back to it.
 *   Returns 0 if the message was handled and responded correctly, or -1 if an error occurred.
 */
int _handle_client_socket_pollin_event(int client_socket) {
  struct message_t* message = network_receive(client_socket);
  if (message == NULL) {
    return -1;
  }
  logger_info__sockfd(client_socket, "Handling message from client...");
  if (invoke(message) < 0) {
    message_destroy(message);
    return -1;
  }
  int send_result = network_send(client_socket, message);
  message_destroy(message);

  return (send_result < 0) ? -1 : 0;
}

/* Handles the events in the given client sockets. */
void _handle_client_sockets_events(struct pollfd* sockets, int* client_sockets_count) {
  int some_fds_closed = 0;

  for (int i = 1; i <= *client_sockets_count; ++i) {
    int client_socket_fd = sockets[i].fd;
    if (client_socket_fd >= 0 && sockets[i].revents & POLLIN) {
      int handler_result = _handle_client_socket_pollin_event(client_socket_fd);
      if (handler_result < 0) {
        close(client_socket_fd);
        logger_info__sockfd(client_socket_fd, "Closed client connection");
        sockets[i].fd = -1;

        some_fds_closed = 1;
      }
    }
  }

  if (some_fds_closed) {
    // move pollfds that were closed to the end of the array (because poll does not notify about
    // revents of pollfds that are after those with fd -1)

    int index_of_last_connected = MAX_CLIENT_SOCKETS;
    for (int i = MAX_CLIENT_SOCKETS; i >= 1; --i) {
      if (sockets[i].fd != -1) {
        index_of_last_connected = i;
        break;
      }
    }
    for (int i = index_of_last_connected - 1; i >= 1; --i) {
      if (sockets[i].fd == -1) {
        struct pollfd socket_to_move = sockets[i];
        for (int j = i + 1; j <= index_of_last_connected; ++j) {
          sockets[j - 1] = sockets[j];
        }
        sockets[index_of_last_connected] = socket_to_move;
        index_of_last_connected -= 1;
      }
    }

    *client_sockets_count = index_of_last_connected;
  }
}

int network_main_loop(int listening_socket) {
  struct pollfd sockets[1 + MAX_CLIENT_SOCKETS];
  sockets[0].fd = listening_socket;
  sockets[0].events = POLLIN;
  sockets[0].revents = 0;
  for (int i = 1; i <= MAX_CLIENT_SOCKETS; i++) {
    sockets[i].fd = -1;
    sockets[i].events = POLLIN;
    sockets[i].revents = 0;
  }
  int client_sockets_count = 0;

  while (1) {
    int num_of_sockets_with_events = poll(sockets, 1 + client_sockets_count, -1);

    if (num_of_sockets_with_events < 0) {
      logger_perror("network_main_loop", "Poll failed");
      _close_all_client_sockets(sockets);
      return -1;
    } else if (num_of_sockets_with_events == 0) {
      // this is not expected to happen because we are not setting a timeout.
      // simply log it and proceed
      logger_error("network_main_loop", "Unexpected timeout");
    } else {
      _handle_listening_socket_events(sockets, &client_sockets_count);
      _handle_client_sockets_events(sockets, &client_sockets_count);
    }
  }

  return 0;
}

int network_server_close() {
  tree_skel_destroy();
  logger_info("Closed server\n");
  return 0;
}
