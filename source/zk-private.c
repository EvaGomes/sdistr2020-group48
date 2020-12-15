/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "logger-private.h"

#include <unistd.h>
#include <zookeeper/zookeeper.h>

static const char* ROOT_ZNODE_PATH = "/kvstore";
static zhandle_t* zh;
static int is_connected;

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
  if (ZNONODE == zoo_exists(zh, ROOT_ZNODE_PATH, 0, NULL)) {
    if (ZOK == zoo_create(zh, ROOT_ZNODE_PATH, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
      logger_debug("Created Zookeeper's root znode\n");
    } else {
      logger_perror("zk_connect", "Failed to create zookeeper's root znode");
      return -1;
    }
  }
  return 0;
}

void zk_close() {
  zookeeper_close(zh);
}
