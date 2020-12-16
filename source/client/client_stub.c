/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "inet-private.h"
#include "logger-private.h"
#include "message-private.h"
#include "network_client.h"
#include "zk-private.h"

#include <stdlib.h>
#include <string.h>

#define SIZE_OF_RTREE sizeof(struct rtree_t)

static int are_servers_connected = 0;

void _rtree_destroy(struct rtree_t* rtree) {
  if (rtree != NULL) {
    free(rtree->primary_server_ip_address);
    free(rtree->backup_server_ip_address);
    free(rtree);
  }
}

int servers_retry_connect(struct rtree_t* rtree) {
  char* primary_address_port = zk_get_primary_tree_server();
  char* backup_address_port = zk_get_backup_tree_server();

  if ((primary_address_port == NULL) || (backup_address_port == NULL) ||
      parse_address_port(primary_address_port, &(rtree->primary_server_ip_address),
                         &(rtree->primary_server_port)) < 0 ||
      parse_address_port(backup_address_port, &(rtree->backup_server_ip_address),
                         &(rtree->backup_server_port)) < 0 ||
      network_connect(rtree) < 0) {
    free(primary_address_port);
    free(backup_address_port);
    return -1;
  }

  free(primary_address_port);
  free(backup_address_port);
  are_servers_connected = 1;
  return 0;
}

static void on_servers_changed(void* listener_context) {
  logger_debug("Function 'on_servers_changed' was invoked\n");
  struct rtree_t* rtree = (struct rtree_t*) listener_context;
  network_close(rtree);
  are_servers_connected = 0;
}

struct rtree_t* rtree_connect(const char* address_port) {
  if (address_port == NULL) {
    logger_error_invalid_arg("rtree_connect", "address_port", address_port);
    return NULL;
  }
  if (zk_connect(address_port) < 0) {
    zk_close();
    return NULL;
  }

  struct rtree_t* rtree = malloc(SIZE_OF_RTREE);
  if (rtree == NULL) {
    logger_error_malloc_failed("rtree_connect");
    return NULL;
  }

  servers_retry_connect(rtree);

  zk_register_servers_listener(on_servers_changed, rtree);
  return rtree;
}

int rtree_are_servers_connected() {
  return are_servers_connected;
}

int rtree_disconnect(struct rtree_t* rtree) {
  int close_result = network_close(rtree);
  are_servers_connected = 0;
  zk_close();
  _rtree_destroy(rtree);
  return (close_result < 0) ? -1 : 0;
}

int rtree_put(struct rtree_t* rtree, struct entry_t* entry) {
  if (entry == NULL) {
    logger_error_invalid_arg("rtree_put", "entry", "NULL");
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

  if (response == NULL || response->msg->op_code == OP_ERROR ||
      response->msg->content_case != CT_OP_ID) {
    message_destroy(response);
    return -1;
  }
  int op_id = response->msg->op_id;
  message_destroy(response);
  return op_id;
}

struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
  if (key == NULL) {
    logger_error_invalid_arg("rtree_get", "key", key);
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
    logger_error_invalid_arg("rtree_del", "key", "NULL");
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

  if (response == NULL || response->msg->op_code == OP_ERROR ||
      response->msg->content_case != CT_OP_ID) {
    message_destroy(response);
    return -1;
  }
  int op_id = response->msg->op_id;
  message_destroy(response);
  return op_id;
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
    return -2;
  }
  request->msg->op_code = OP_HEIGHT;
  request->msg->content_case = CT_NONE;

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL) {
    return -2;
  }
  if (response->msg->op_code == OP_ERROR || response->msg->content_case != CT_INT_RESULT) {
    message_destroy(response);
    return -2;
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

int rtree_verify(struct rtree_t* rtree, int op_id) {
  struct message_t* request = message_create();
  if (request == NULL) {
    return -1;
  }
  request->msg->op_code = OP_VERIFY;
  request->msg->content_case = CT_OP_ID;
  request->msg->op_id = op_id;

  struct message_t* response = network_send_receive(rtree, request);
  message_destroy(request);

  if (response == NULL || response->msg->op_code == OP_ERROR ||
      response->msg->content_case != CT_INT_RESULT) {
    message_destroy(response);
    return -1;
  }
  int op_result = response->msg->int_result;
  message_destroy(response);
  return op_result;
}

void rtree_free_keys(char** keys) {
  logger_error("rtree_free_keys", "Not implemented");
}
