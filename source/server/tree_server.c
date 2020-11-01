/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "inet-private.h"
#include "network_server.h"
#include "tree_skel.h"

#include <errno.h>

int main(int argc, char** argv) {

  ignore_SIGPIPE_signals();

  if (argc != 2) {
    errno = EINVAL;
    fprintf(stderr, "Invalid number of arguments\n");
    printf("Usage: ./tree_server <port>\n");
    printf(" E.g.: ./tree_server 9000\n");
    return -1;
  }

  int port = htons(atoi(argv[1]));
  if (port == 0) {
    errno = EINVAL;
    fprintf(stderr, "Invalid argument \"%s\"\n", argv[1]);
    return -1;
  }
  
    int listening_socket = network_server_init(port);
    if (listening_socket < 0) {
      return -1;
    }

    if (network_main_loop(listening_socket) < 0) {
        fprintf(stderr, "Unexpected error in network_main_loop");
    }

    network_server_close();
    return close(listening_socket);
}
