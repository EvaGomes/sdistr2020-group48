/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet.h"
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

int network_main_loop(int listening_socket) {

  int connsockfd;
  struct sockaddr_in client;
  char str[MAX_MSG + 1];
  int nbytes, count;
  socklen_t size_client;

  // accept bloqueia à espera de pedidos de conexão.
  // Quando retorna já foi feito o "three-way handshake" e connsockfd é uma
  // socket pronta a comunicar com o cliente.
  while ((connsockfd = accept(listening_socket, (struct sockaddr*) &client, &size_client)) != -1) {

    printf("Accepted client connection; connsockfd=%d", connsockfd);

    // Lê string (no máximo MAX_MSG bytes) enviada pelo cliente
    // através da socket
    if ((nbytes = read(connsockfd, str, MAX_MSG)) < 0) {
      perror("Error while reading request-data from client");
      close(connsockfd);
      continue;
    }

    printf("Got data from client");

    // Coloca terminador de string
    str[nbytes] = '\0';
    // Conta número de carateres
    count = strlen(str);
    // Converte count para formato de rede
    count = htonl(count);

    // Envia tamanho da string ao cliente através da socket
    if ((nbytes = write(connsockfd, &count, sizeof(count))) != sizeof(count)) {
      perror("Error while sending response-data to the client");
      close(connsockfd);
      continue;
    }

    // Fecha socket referente a esta conexão
    close(connsockfd);
  }
  return 0; // TODO
}

struct message_t* network_receive(int client_socket) {
  return 0; // TODO
}

int network_send(int client_socket, struct message_t* msg) {
  return 0; // TODO
}

int network_server_close() {
  tree_skel_destroy();
  return 0;
}
