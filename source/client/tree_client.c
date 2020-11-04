/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "inet-private.h"
#include "network_client.h"

#include <errno.h>
#include <signal.h>

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
      free(input_str);
      break;
    }

    char* command = input_str; // TODO: parse input string
    char *key, data;

    if (strcmp(command, "put") == 0) {
      printf("// TODO parse input_str and get 'key' and 'data'\n");
    } else if (strcmp(command, "get") == 0) {
      printf("// TODO parse input_str and get 'key'\n");
      // char* value = rtree_get(rtree, key);
      // if (value == NULL) {
      //   printf("Failed!");
      // } else {
      //   printf("value: %s", value);
      // }
    } else if (strcmp(command, "del") == 0) {
      printf("// TODO parse input_str and get 'key'\n");
      // if (rtree_del(rtree, key) < 0) {
      //   printf("Failed!");
      // } else {
      //   printf("Done!");
      // }
    } else if (strcmp(input_str, "size") == 0) {
      printf("size: %d\n", rtree_size(rtree));
    } else if (strcmp(input_str, "height") == 0) {
      printf("height: %d\n", rtree_height(rtree));
    } else if (strcmp(input_str, "getkeys") == 0) {
      char** keys = rtree_get_keys(rtree);
      if (keys == NULL) {
        printf("Failed!\n");
      } else {
        printf("keys: // TODO print keys\n");
      }
    } else {
      printf("unknown command!\n");
    }
  }

  printf("Quitting...");
  return rtree_disconnect(rtree);
}
