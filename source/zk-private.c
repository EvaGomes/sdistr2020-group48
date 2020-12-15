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
#include <zookeeper/zookeeper.h>

static zhandle_t* zh;
static int is_connected;
static const char* ROOT_ZNODE_PATH = "/kvstore";
static const char* PRIMARY_TREE_SERVER_ZNODE_PATH = "/kvstore/primary";
static const char* BACKUP_TREE_SERVER_ZNODE_PATH = "/kvstore/backup";

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
  int valuelen = (value == NULL) ? -1 : (int) strlen(value);
  return zoo_create(zh, path, value, valuelen, &ZOO_OPEN_ACL_UNSAFE, mode, NULL, 0);
}

int zk_connect(char* zookeeper_address_and_port) {
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
  return 0;
}

enum ServerRole zk_register_tree_server(int server_listening_socket_fd) {

  // determine IP adress and port from socket descriptor

  struct sockaddr_in server_sa;
  socklen_t server_sa_len = sizeof(server_sa);
  if (getsockname(server_listening_socket_fd, (struct sockaddr*) &server_sa, &server_sa_len) < 0) {
    logger_perror("zk_register_tree_server", "Failed to get socket's name");
    return -1;
  }
  char* server_ip_address = inet_ntoa(server_sa.sin_addr);
  int server_port = (int) ntohs(server_sa.sin_port);
  char server_address_and_port[16 + 1 + 5];
  sprintf(server_address_and_port, "%s:%d", server_ip_address, server_port);

  // create appropriate znode

  if (ZNONODE == zk_exists(PRIMARY_TREE_SERVER_ZNODE_PATH)) {
    if (ZOK == zk_create(PRIMARY_TREE_SERVER_ZNODE_PATH, server_address_and_port, ZOO_EPHEMERAL)) {
      logger_debug("Registered %s as the primary tree_server\n", server_address_and_port);
      return PRIMARY;
    } else {
      logger_perror("zk_register_tree_server", "Failed to create znode with path %s",
                    PRIMARY_TREE_SERVER_ZNODE_PATH);
      return NONE;
    }
  }
  if (ZNONODE == zk_exists(BACKUP_TREE_SERVER_ZNODE_PATH)) {
    if (ZOK == zk_create(BACKUP_TREE_SERVER_ZNODE_PATH, server_address_and_port, ZOO_EPHEMERAL)) {
      logger_debug("Registered %s as the backup tree_server\n", server_address_and_port);
      return BACKUP;
    } else {
      logger_perror("zk_register_tree_server", "Failed to create znode with path %s",
                    BACKUP_TREE_SERVER_ZNODE_PATH);
      return NONE;
    }
  }
  logger_error("zk_register_tree_server",
               "Failed to register %s, primary and backup tree_servers are already set",
               server_address_and_port);
  return NONE;
}

void zk_close() {
  zookeeper_close(zh);
}
