/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "message-private.h"
#include "network_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_OF_RTREE sizeof(struct rtree_t)

void _rtree_destroy(struct rtree_t* rtree) {
  if (rtree != NULL) {
    free(rtree->server_ip_address);
    free(rtree);
  }
}

struct rtree_t* rtree_connect(const char* address_port) {
  if (address_port == NULL) {
    fprintf(stderr, "\nERR: rtree_connect: invalid address_port \"%s\"\n", address_port);
    return NULL;
  }

  // remove const to prevent -Wdiscarded-qualifiers in strtok
  char address_and_port[strlen(address_port)];
  strcpy(address_and_port, address_port);

  char* delimiter = ":";
  char* ip_adress = strtok(address_and_port, delimiter);
  char* port_str = strtok(NULL, delimiter);

  if (port_str == NULL) {
    fprintf(stderr, "\nERR: rtree_connect: invalid address_port \"%s\"\n", address_port);
    return NULL;
  }
  int port = atoi(port_str);
  if (port == 0) {
    fprintf(stderr, "\nERR: rtree_connect: invalid address_port \"%s\"\n", address_port);
    return NULL;
  }

  struct rtree_t* rtree = malloc(SIZE_OF_RTREE);
  if (rtree == NULL) {
    fprintf(stderr, "\nERR: rtree_connect: malloc failed\n");
    return NULL;
  }
  rtree->server_ip_address = strdup(ip_adress);
  rtree->server_port = port;

  if (network_connect(rtree) < 0) {
    _rtree_destroy(rtree);
    return NULL;
  }

  return rtree;
}

int rtree_disconnect(struct rtree_t* rtree) {
  int close_result = network_close(rtree);
  _rtree_destroy(rtree);
  return (close_result < 0) ? -1 : 0;
}

int rtree_put(struct rtree_t* rtree, struct entry_t* entry) {
  if (entry == NULL) {
    fprintf(stderr, "\nERR: rtree_put: invalid arg entry\n");
    return -1;
  }

  struct message_t* request = message_create();
  if (request == NULL) {
    return -1;
  }
  request->msg->op_code = OP_PUT;
  request->msg->content_case = CT_ENTRY;
  request->msg->entry = entry_to_msg(entry);

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL || response->msg->op_code == OP_ERROR) {
    return -1;
  }
  message_destroy(response);
  return 0;
}

struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
  if (key == NULL) {
    fprintf(stderr, "\nERR: rtree_get: invalid arg key\n");
    return NULL;
  }

  struct message_t* request = message_create();
  if (request == NULL) {
    return NULL;
  }
  request->msg->op_code = OP_GET;
  request->msg->content_case = CT_KEY;
  request->msg->key = strdup(key);

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return NULL;
  }
  if (response->msg->op_code == OP_ERROR || response->msg->content_case != CT_VALUE) {
    message_destroy(response);
    return NULL;
  }
  struct data_t* ret = msg_to_data(response->msg->value);
  message_destroy(response);
  return ret;
}

int rtree_del(struct rtree_t* rtree, char* key) {
  if (key == NULL) {
    fprintf(stderr, "\nERR: rtree_del: invalid arg key\n");
    return -1;
  }

  struct message_t* request = message_create();
  if (request == NULL) {
    return -1;
  }
  request->msg->op_code = OP_DEL;
  request->msg->content_case = CT_KEY;
  request->msg->key = strdup(key);

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return -1;
  }
  if (response == NULL || response->msg->op_code == OP_ERROR) {
    message_destroy(response);
    return -1;
  }
  message_destroy(response);
  return 0;
}

int rtree_size(struct rtree_t* rtree) {
  struct message_t* request = message_create();
  if (request == NULL) {
    return -1;
  }
  request->msg->op_code = OP_SIZE;
  request->msg->content_case = CT_NONE;

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return -1;
  }
  if (response->msg->op_code == OP_ERROR || response->msg->content_case != CT_INT_RESULT) {
    message_destroy(response);
    return -1;
  }
  int ret = response->msg->int_result;
  message_destroy(response);
  return ret;
}

int rtree_height(struct rtree_t* rtree) {
  struct message_t* request = message_create();
  if (request == NULL) {
    return -1;
  }
  request->msg->op_code = OP_HEIGHT;
  request->msg->content_case = CT_NONE;

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return -1;
  }
  if (response->msg->op_code == OP_ERROR || response->msg->content_case != CT_INT_RESULT) {
    message_destroy(response);
    return -1;
  }
  int ret = response->msg->int_result;
  message_destroy(response);
  return ret;
}

char** rtree_get_keys(struct rtree_t* rtree) {
  struct message_t* request = message_create();
  if (request == NULL) {
    return NULL;
  }
  request->msg->op_code = OP_GETKEYS;
  request->msg->content_case = CT_NONE;

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return NULL;
  }
  if (response->msg->op_code == OP_ERROR || response->msg->content_case != CT_KEYS) {
    message_destroy(response);
    return NULL;
  }
  char** ret = msg_to_keys(response->msg->keys);
  message_destroy(response);
  return ret;
}

void rtree_free_keys(char** keys) {
  // TODO
}
