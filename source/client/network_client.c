/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "client_stub.h"
#include "inet.h"
#include "sdmessage.pb-c.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t* rtree) {

  if ((rtree->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating TCP socket");
    return -1;
  }

  short server_port = rtree->server_port;
  char* server_ip_address = rtree->server_ip_address;
  int sockfd = rtree->sockfd;

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(server_port);

  if (inet_pton(AF_INET, server_ip_address, &server.sin_addr) < 1) {
    fprintf(stderr, "Error converting server's IP address \"%s\"", server_ip_address);
    close(sockfd);
    return -1;
  }

  if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
    fprintf(stderr, "Error connecting to server at %s:%d", server_ip_address, server_port);
    close(sockfd);
    return -1;
  }

  printf("\nConnected to server at %s:%d...\n", rtree->server_ip_address, rtree->server_port);
  return 0;
}

struct message_t* network_send_receive(struct rtree_t* rtree, struct message_t* msg) {
  return 0; // TODO
}

int network_close(struct rtree_t* rtree) {
  return close(rtree->sockfd);
}
