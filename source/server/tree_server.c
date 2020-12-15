/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "logger-private.h"
#include "network_server.h"
#include "zk-private.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handle_SIGINT_signal(int signal) {
  logger_debug("Caught signal %d\n", signal);
}

static int validate_args(int argc, char** argv) {
  if (argc != 3) {
    errno = EINVAL;
    fprintf(stderr, "Invalid number of arguments\n");
    printf("Usage: ./tree_server <serverPort> <zookeeperIP>:<zookeeperPort>\n");
    printf(" E.g.: ./tree_server 9000 127.0.0.1:2181\n");
    return -1;
  }

  int serverPort = htons(atoi(argv[1]));
  if (serverPort <= 0) {
    errno = EINVAL;
    fprintf(stderr, "Invalid argument \"%s\"\n", argv[1]);
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {

  signal(SIGINT, handle_SIGINT_signal);
  signal(SIGPIPE, SIG_IGN);

  if (validate_args(argc, argv) < 0) {
    return -1;
  }

  int serverPort = htons(atoi(argv[1]));
  char* zookeeper_address_and_port = argv[2];

  if (zk_connect(zookeeper_address_and_port) < 0) {
    zk_close();
    return -1;
  }
  int listening_socket = network_server_init(serverPort);
  if (listening_socket < 0) {
    zk_close();
    return -1;
  }

  network_main_loop(listening_socket);

  close(listening_socket);
  network_server_close();
  zk_close();
}
