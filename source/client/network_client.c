/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "inet-private.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int network_connect(struct rtree_t* rtree) {
  if (rtree == NULL) {
    fprintf(stderr, "\nERR: network_connect: invalid rtree\n");
    return -1;
  }

  if ((rtree->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating TCP socket\n");
    return -1;
  }

  short server_port = rtree->server_port;
  char* server_ip_address = rtree->server_ip_address;
  int sockfd = rtree->sockfd;

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(server_port);

  if (inet_pton(AF_INET, server_ip_address, &server.sin_addr) < 1) {
    fprintf(stderr, "Error converting server's IP address \"%s\"\n", server_ip_address);
    close(sockfd);
    return -1;
  }

  if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
    fprintf(stderr, "Error connecting to server at %s:%d\n", server_ip_address, server_port);
    close(sockfd);
    return -1;
  }

  printf("\nConnected to server at %s:%d...\n", rtree->server_ip_address, rtree->server_port);
  return 0;
}

struct message_t* network_send_receive(struct rtree_t* rtree, struct message_t* msg) {
  if (rtree == NULL || msg == NULL) {
    fprintf(stderr, "\nERR: network_send_receive: invalid args\n");
    return NULL;
  }

  int sockfd = rtree->sockfd;

  if (network_send_message(sockfd, msg) < 0) {
    return NULL;
  }
  return network_receive_message(sockfd);
}

int network_close(struct rtree_t* rtree) {
  if (rtree == NULL) {
    fprintf(stderr, "\nERR: network_close: invalid rtree\n");
    return -1;
  }
  return close(rtree->sockfd);
}
