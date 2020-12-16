/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "logger-private.h"
#include "server_role-private.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <zk-private.h>
#include <zookeeper/zookeeper.h>

/* The maximum length of an "<ip-address>:<port>" string. */
#define SERVER_ADDRESS_AND_PORT_MAX_LEN 16 + 1 + 5 + 1

static zhandle_t* zh;
static int is_connected;
static const char* ROOT_ZNODE_PATH = "/kvstore";
static const char* PRIMARY_TREE_SERVER_ZNODE_PATH = "/kvstore/primary";
static const char* BACKUP_TREE_SERVER_ZNODE_PATH = "/kvstore/backup";

enum ServerRole server_role = NONE;
servers_listener_fn servers_listener = NULL;
void* servers_listener_context = NULL;

/* Watcher function for connection state change events. */
static void zk_connection_watcher(zhandle_t* zzh, int type, int state, const char* path,
                                  void* context) {
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      logger_debug("zk_connection_watcher got state ZOO_CONNECTED_STATE...\n");
      is_connected = 1;
    }
  }
}

static ZOOAPI int zk_exists(const char* path) {
  return zoo_exists(zh, path, 0, NULL);
}

static ZOOAPI int zk_create(const char* path, const char* value, int mode) {
  int valuelen = (value == NULL) ? -1 : ((int) strlen(value) + 1);
  return zoo_create(zh, path, value, valuelen, &ZOO_OPEN_ACL_UNSAFE, mode, NULL, 0);
}

static void String_vector_destroy(struct String_vector* vector) {
  for (int i = 0; i < vector->count; ++i) {
    free(vector->data[i]);
  }
  free(vector->data);
  free(vector);
}

static void zk_children_watcher(zhandle_t* zzh, int type, int state, const char* path,
                                void* context) {
  struct String_vector* children = malloc(sizeof(struct String_vector));
  if (type == ZOO_CHILD_EVENT && state == ZOO_CONNECTED_STATE) {
    if (ZOK != zoo_wget_children(zh, ROOT_ZNODE_PATH, zk_children_watcher, context, children)) {
      logger_error("zk_children_watcher", "Failed to set children watcher");
    }
    if (servers_listener != NULL) {
      servers_listener(servers_listener_context);
    }
  }
  String_vector_destroy(children);
}

int zk_connect(const char* zookeeper_address_and_port) {
  zh = zookeeper_init(zookeeper_address_and_port, zk_connection_watcher, 2000, 0, 0, 0);
  if (zh == NULL) {
    logger_perror("zk_connect", "Failed to connect to ZooKeeper server!");
    return -1;
  }

  // wait at most 5 seconds for state to change to CONNECTED
  for (int i = 1; i <= 5; ++i) {
    if (is_connected) {
      break;
    } else {
      sleep(1);
      logger_debug("Waiting for Zookeeper server to connect... (attempt#%d)\n", i);
    }
  }
  if (!is_connected) {
    logger_error("zk_connect", "Failed to connect to ZooKeeper server!");
    return -1;
  }

  // create root-node
  if (ZNONODE == zk_exists(ROOT_ZNODE_PATH)) {
    if (ZOK == zk_create(ROOT_ZNODE_PATH, NULL, 0)) {
      logger_debug("Created Zookeeper's root znode\n");
    } else {
      logger_perror("zk_connect", "Failed to create zookeeper's root znode");
      return -1;
    }
  }

  // register root's children listener
  struct String_vector* children = malloc(sizeof(struct String_vector));
  if (ZOK != zoo_wget_children(zh, ROOT_ZNODE_PATH, zk_children_watcher, "", children)) {
    logger_error("zk_children_watcher", "Failed to set children watcher");
  }
  String_vector_destroy(children);

  logger_info("\nConnected to zookeeper server at %s...\n", zookeeper_address_and_port);
  return 0;
}

int zk_register_tree_server(int server_listening_socket_fd) {

  // determine IP adress and port from socket descriptor

  struct sockaddr_in server_sa;
  socklen_t server_sa_len = sizeof(server_sa);
  if (getsockname(server_listening_socket_fd, (struct sockaddr*) &server_sa, &server_sa_len) < 0) {
    logger_perror("zk_register_tree_server", "Failed to get socket's name");
    return -1;
  }
  char* server_ip_address = inet_ntoa(server_sa.sin_addr);
  int server_port = (int) ntohs(server_sa.sin_port);
  char server_address_and_port[SERVER_ADDRESS_AND_PORT_MAX_LEN];
  sprintf(server_address_and_port, "%s:%d", server_ip_address, server_port);

  // create appropriate znode

  if (ZNONODE == zk_exists(PRIMARY_TREE_SERVER_ZNODE_PATH)) {
    if (ZOK == zk_create(PRIMARY_TREE_SERVER_ZNODE_PATH, server_address_and_port, ZOO_EPHEMERAL)) {
      logger_debug("Registered %s as the primary tree_server\n", server_address_and_port);
      server_role = PRIMARY;
      return 0;
    } else {
      logger_perror("zk_register_tree_server", "Failed to create znode with path %s",
                    PRIMARY_TREE_SERVER_ZNODE_PATH);
      return -1;
    }
  }
  if (ZNONODE == zk_exists(BACKUP_TREE_SERVER_ZNODE_PATH)) {
    if (ZOK == zk_create(BACKUP_TREE_SERVER_ZNODE_PATH, server_address_and_port, ZOO_EPHEMERAL)) {
      logger_debug("Registered %s as the backup tree_server\n", server_address_and_port);
      server_role = BACKUP;
      return 0;
    } else {
      logger_perror("zk_register_tree_server", "Failed to create znode with path %s",
                    BACKUP_TREE_SERVER_ZNODE_PATH);
      return -1;
    }
  }
  logger_error("zk_register_tree_server",
               "Failed to register %s, primary and backup tree_servers are already set",
               server_address_and_port);
  return -1;
}

enum ServerRole zk_get_tree_server_role() {
  return server_role;
}

/* Returns the "<ip-address>:<port>" of the primary tree_server, or NULL if it isn't registered. */
char* zk_get_primary_tree_server() {
  char server[SERVER_ADDRESS_AND_PORT_MAX_LEN] = "";
  int server_len = SERVER_ADDRESS_AND_PORT_MAX_LEN;
  if (zoo_get(zh, PRIMARY_TREE_SERVER_ZNODE_PATH, 0, server, &server_len, NULL) < 0) {
    return NULL;
  }
  return strdup(server);
}

/* Returns the "<ip-address>:<port>" of the backup tree_server, or NULL if it isn't registered. */
char* zk_get_backup_tree_server() {
  char server[SERVER_ADDRESS_AND_PORT_MAX_LEN] = "";
  int server_len = SERVER_ADDRESS_AND_PORT_MAX_LEN;
  if (zoo_get(zh, BACKUP_TREE_SERVER_ZNODE_PATH, 0, server, &server_len, NULL) < 0) {
    return NULL;
  }
  return strdup(server);
}

void zk_register_servers_listener(servers_listener_fn listener, void* context) {
  servers_listener = listener;
  servers_listener_context = context;
}

int zk_update_server_roles() {
  if (ZNONODE == zk_exists(PRIMARY_TREE_SERVER_ZNODE_PATH) &&
      ZOK == zk_exists(BACKUP_TREE_SERVER_ZNODE_PATH)) {

    char new_primary_server[SERVER_ADDRESS_AND_PORT_MAX_LEN];
    int new_primary_server_len = SERVER_ADDRESS_AND_PORT_MAX_LEN;
    if (zoo_get(zh, BACKUP_TREE_SERVER_ZNODE_PATH, 0, new_primary_server, &new_primary_server_len,
                NULL) < 0) {
      logger_perror("zk_update_server_roles", "Failed to get data of znode with path %s",
                    PRIMARY_TREE_SERVER_ZNODE_PATH);
      return -1;
    }

    if (ZOK == zk_create(PRIMARY_TREE_SERVER_ZNODE_PATH, new_primary_server, ZOO_EPHEMERAL)) {
      logger_debug("Registered %s as the primary tree_server\n", new_primary_server);
    } else {
      logger_perror("zk_update_server_roles", "Failed to create znode with path %s",
                    PRIMARY_TREE_SERVER_ZNODE_PATH);
      return -1;
    }

    if (server_role == BACKUP) {
      logger_debug("This server has been promoted to primary server.\n");
      server_role = PRIMARY;
    }

    if (zoo_delete(zh, BACKUP_TREE_SERVER_ZNODE_PATH, 0) < 0) {
      logger_perror("zk_update_server_roles", "Failed to delete znode with path %s",
                    BACKUP_TREE_SERVER_ZNODE_PATH);
      return -1;
    }
  }
  return 0;
}

void zk_close() {
  server_role = NONE;
  servers_listener_context = NULL;
  servers_listener = NULL;
  zookeeper_close(zh);
}
