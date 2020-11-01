/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "inet.h"
#include "network_client.h"

#include <errno.h>

int _test_args(int argc) {
  if (argc != 3) { // TODO: change to 2
    errno = EINVAL;
    strerror(errno);
    printf("Usage: ./tree_client <server-ip>:<server-port> <str>\n");
    printf(" E.g.: ./tree_client 127.0.0.1:9000 some_str-value\n");
    return -1;
  }
  return 0;
}

int main(int argc, char** argv) {

  if (_test_args(argc) < 0) {
    return -1;
  }

  struct rtree_t* rtree = rtree_connect(argv[1]);
  if (rtree == NULL) {
    return -1;
  }
  
  int sockfd = rtree->sockfd;

  // Copia os primeiros bytes da string (no máximo MAX_MSG-1) passada como argumento
  char str[MAX_MSG];
  strncpy(str, argv[2], MAX_MSG - 1);
  // Garante que a string tem terminação. Se era maior que MAX_MSG será truncada.
  str[MAX_MSG - 1] = '\0';

  int nbytes;

  // Envia string
  if ((nbytes = write(sockfd, str, strlen(str))) != strlen(str)) {
    perror("Erro ao enviar dados ao servidor");
    close(sockfd);
    return -1;
  }

  printf("À espera de resposta do servidor ...\n");

  // Recebe tamanho da string
  int count;
  if ((nbytes = read(sockfd, &count, sizeof(count))) != sizeof(count)) {
    perror("Erro ao receber dados do servidor");
    close(sockfd);
    return -1;
  };

  printf("O tamanho da string é %d!\n", ntohl(count));

  return rtree_disconnect(rtree);
}
