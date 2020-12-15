/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

struct rtree_t {
  char* primary_server_ip_address;
  int primary_server_port;
  int primary_sockfd;

  char* backup_server_ip_address;
  int backup_server_port;
  int backup_sockfd;
};

int rtree_are_servers_connected();
int servers_retry_connect(struct rtree_t* rtree);

#endif
