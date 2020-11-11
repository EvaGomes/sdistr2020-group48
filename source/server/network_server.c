/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "message-private.h"
#include "tree_skel.h"

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int network_server_init(short port) {
  int listening_socket;

  if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating TCP socket");
    return -1;
  }

  int enable = 1;
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("Failed to allow reuse of local addresses");
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = port;
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listening_socket, (struct sockaddr*) &server, sizeof(server)) < 0) {
    perror("Failed to bind the socket to the local address");
    close(listening_socket);
    return -1;
  }

  if (listen(listening_socket, 0) < 0) {
    perror("Failed to prepare socket for listening");
    close(listening_socket);
    return -1;
  }

  if (tree_skel_init() < 0) {
    perror("Failed to init the binary tree...");
    close(listening_socket);
    return -1;
  }

  printf("\nServer is ready...\n");
  return listening_socket;
}

struct message_t* network_receive(int client_socket) {
  return network_receive_message(client_socket);
}

int network_send(int client_socket, struct message_t* msg) {
  return network_send_message(client_socket, msg);
}

int _client_connection_loop(int connsockfd) {
  while (1) {

    struct message_t* message = network_receive(connsockfd);
    if (message == NULL) {
      return -1;
    }
    if (invoke(message) < 0) {
      message_destroy(message);
      return -1;
    }
    int send_result = network_send(connsockfd, message);
    message_destroy(message);
    if (send_result < 0) {
      return -1;
    }
  }
}

int network_main_loop(int listening_socket) {

  int connsockfd;
  struct sockaddr_in client;
  socklen_t size_client = 0;

  while ((connsockfd = accept(listening_socket, (struct sockaddr*) &client, &size_client)) != -1) {
    printf("sockfd=%d - Accepted client connection\n", connsockfd);
    _client_connection_loop(connsockfd);
    close(connsockfd);
    printf("sockfd=%d - Closed client connection\n", connsockfd);
  }
  return 0;
}

int network_server_close() {
  tree_skel_destroy();
  return 0;
}
