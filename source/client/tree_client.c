/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "inet.h"
#include "network_client.h"

#include <errno.h>
#include <signal.h>

int ignore_SIGPIPE_signals() {
  struct sigaction s;
  s.sa_handler = SIG_IGN;
  if (sigaction(SIGPIPE, &s, NULL) != 0) {
    fprintf(stderr, "Failed to ignore SIGPIPE signals\n");
    return -1;
  }
  return 0;
}

char* collect_input() {

  char* input_str = malloc(MAX_MSG * sizeof(char));
  printf("> ");
  fgets(input_str, MAX_MSG, stdin);

  // prune: free over occupied space and drop \n at the end
  int input_str_len = strlen(input_str);
  char last_char = input_str[input_str_len - 1];
  int str_len = (last_char == '\n') ? (input_str_len - 1) : input_str_len;
  char* str = strndup(input_str, str_len);
  free(input_str);

  return str;
}

int main(int argc, char** argv) {

  ignore_SIGPIPE_signals();

  if (argc != 2) {
    errno = EINVAL;
    fprintf(stderr, "Invalid number of arguments\n");
    printf("Usage: ./tree_client <server-ip>:<server-port>\n");
    printf(" E.g.: ./tree_client 127.0.0.1:9000\n");
    return -1;
  }

  struct rtree_t* rtree = rtree_connect(argv[1]);
  if (rtree == NULL) {
    return -1;
  }

  int sockfd = rtree->sockfd;

  while (1) {

    char* input_str = collect_input();
    int input_str_len = strlen(input_str);
    
    if (strcmp(input_str, "quit") == 0) {
      break;
    }

    int nbytes;

    if ((nbytes = write(sockfd, input_str, input_str_len)) != input_str_len) {
      fprintf(stderr, "Error while sending request-data to server\n  data=%s\n", input_str);
      free(input_str);
      continue;
    }

    free(input_str);
    printf("Waiting for a server response...\n");

    int count;
    if ((nbytes = read(sockfd, &count, sizeof(count))) != sizeof(count)) {
      fprintf(stderr, "Error while reading response-data from the server\n");
      continue;
    };

    printf("< Server response: len= %d \n", ntohl(count));
  }

  return rtree_disconnect(rtree);
}
