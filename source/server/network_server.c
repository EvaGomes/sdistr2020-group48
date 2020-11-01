/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "message-private.h"
#include "serialization-private.h"
#include "tree_skel.h"

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
  void* buffer;
  int buffer_size = read(client_socket, buffer, MAX_MSG);

  if (buffer_size < 0) {
    fprintf(stderr, "connsockfd=%d - Error while reading request-data from client\n",
            client_socket);
    return NULL;
  }

  struct message_t* message = buffer_to_message(buffer, buffer_size);
  free(buffer);
  return message;
}

int network_send(int client_socket, struct message_t* msg) {
  void* buffer;
  int buffer_size = message_to_buffer(msg, buffer);
  message_destroy(msg);

  int written_size = write(client_socket, buffer, buffer_size);
  free(buffer);

  if (written_size != buffer_size) {
    fprintf(stderr, "connsockfd=%d - Error while sending response-data to the client\n",
            client_socket);
    return -1;
  }
  return 0;
}

int _client_connection_loop(int connsockfd) {
  char str[MAX_MSG + 1];
  int nbytes, count;

  while (1) {

    struct message_t* message = network_receive(connsockfd);
    if (message == NULL) {
      return -1;
    }

    if (network_send(connsockfd, message) < 0) {
      return -1;
    }

    printf("connsockfd=%d - Sent response-data to client\n", connsockfd);
  }
}

int network_main_loop(int listening_socket) {

  int connsockfd;
  struct sockaddr_in client;
  socklen_t size_client = 0;

  // accept bloqueia à espera de pedidos de conexão.
  // Quando retorna já foi feito o "three-way handshake" e connsockfd é uma
  // socket pronta a comunicar com o cliente.
  while ((connsockfd = accept(listening_socket, (struct sockaddr*) &client, &size_client)) != -1) {

    printf("connsockfd=%d - Accepted client connection\n", connsockfd);
    _client_connection_loop(connsockfd);
    close(connsockfd);
    printf("connsockfd=%d - Closed client connection\n", connsockfd);
  }
  return 0; // TODO
}

int network_server_close() {
  tree_skel_destroy();
  return 0;
}
