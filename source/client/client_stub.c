/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "network_client.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_RTREE = sizeof(struct rtree_t);

void* _on_invalid_arg(const char* address_port) {
    errno = EINVAL;
    fprintf(stderr, "Invalid argument \"%s\"", address_port);
    return NULL;
}

struct rtree_t* rtree_connect(const char* address_port) {
  if (address_port == NULL) {
    return _on_invalid_arg(address_port);
  }
  
  // remove const to prevent -Wdiscarded-qualifiers in strtok
  char address_and_port[strlen(address_port)];
  strcpy(address_and_port, address_port);

  char* delimiter = ":";
  char* ip_adress = strtok(address_and_port, delimiter);
  char* port_str = strtok(NULL, delimiter);

  if (port_str == NULL) {
    return _on_invalid_arg(address_port);
  }
  int port = atoi(port_str);
  if (port == 0) {
    return _on_invalid_arg(address_port);
  }

  struct rtree_t* rtree = malloc(SIZE_OF_RTREE);
  if (rtree == NULL) {
    fprintf(stderr, "nERR: rtree_connect: malloc failed");
    return NULL;
  }
  rtree->server_ip_address = strdup(ip_adress);
  rtree->server_port = port;

  if (network_connect(rtree) < 0) {
    free(rtree);
    return NULL;
  }

  return rtree;
}

int rtree_disconnect(struct rtree_t* rtree) {
  if (network_close(rtree) < 0) {
    return -1;
  }
  free(rtree->server_ip_address);
  free(rtree);
  return 0;
}

int rtree_put(struct rtree_t* rtree, struct entry_t* entry) {
  return 0; // TODO
}

struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
  return 0; // TODO
}

int rtree_del(struct rtree_t* rtree, char* key) {
  return 0; // TODO
}

int rtree_size(struct rtree_t* rtree) {
  return 0; // TODO
}

int rtree_height(struct rtree_t* rtree) {
  return 0; // TODO
}

char** rtree_get_keys(struct rtree_t* rtree) {
  return 0; // TODO
}

void rtree_free_keys(char** keys) {
  // TODO
}
