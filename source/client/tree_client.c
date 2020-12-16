/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "client_stub-private.h"
#include "inet-private.h"
#include "logger-private.h"
#include "network_client.h"
#include "tree.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The maximum length of the string input by the user. */
#define MAX_INPUT_LEN 5000

/* Collects a string, with maximum length MAX_MSG, written by the user in the console. */
char* _collect_input() {

  char* input_str = malloc(MAX_INPUT_LEN * sizeof(char));
  if (input_str == NULL) {
    logger_error_malloc_failed("_collect_input");
    return NULL;
  }
  printf("> ");
  fgets(input_str, MAX_INPUT_LEN, stdin);

  // prune: free over occupied space and drop \n at the end
  size_t input_str_len = strlen(input_str);
  char last_char = input_str[input_str_len - 1];
  int str_len = (last_char == '\n') ? (input_str_len - 1) : input_str_len;
  char* str = strndup(input_str, str_len);
  free(input_str);

  return str;
}

/* Creates an entry_t from the given key and string-data. */
struct entry_t* _entry_create_from_args(char* key, char* data) {
  if (key == NULL || data == NULL) {
    return NULL;
  }
  struct data_t* dataStruct = data_create2(strlen(data) + 1, strdup(data));
  return (dataStruct == NULL) ? NULL : entry_create(strdup(key), dataStruct);
}

/* Prints the given keys to the console. */
void _print_keys(char** keys) {
  printf("< keys: [");
  int index = 0;
  while (keys[index] != NULL) {
    printf("\n    %s", keys[index]);
    index += 1;
  }
  if (index == 0) {
    printf("] (#=0)\n");
  } else {
    printf("\n  ] (#=%d)\n", index);
  }
}

void _run_command(struct rtree_t* rtree, char* command, char* arg1, char* arg2) {

  if (strcmp(command, "size") == 0) {
    int size = rtree_size(rtree);
    if (size < 0) {
      printf("< Query failed!\n");
    } else {
      printf("< size: %d\n", size);
    }
  }

  else if (strcmp(command, "height") == 0) {
    int height = rtree_height(rtree);
    if (height < -1) {
      printf("< Query failed!\n");
    } else {
      printf("< height: %d\n", height);
    }
  }

  else if (strcmp(command, "del") == 0) {
    if (arg1 == NULL) {
      printf("< Invalid args. Usage: del <key>\n");
    } else {
      int op_id = rtree_del(rtree, arg1);
      if (op_id < 0) {
        printf("< Operation failed!\n");
      } else {
        printf("< Operation queued with id %d.\n  (check result with command \"verify %d\")\n",
               op_id, op_id);
      }
    }
  }

  else if (strcmp(command, "get") == 0) {
    if (arg1 == NULL) {
      printf("< Invalid args. Usage: get <key>\n");
    } else {
      struct data_t* value = rtree_get(rtree, arg1);
      if (value == NULL) {
        printf("< Query failed!\n");
      } else if (value->data == NULL) {
        printf("< Key not found!\n");
        data_destroy(value);
      } else {
        printf("< value: %s\n", (char*) value->data);
        data_destroy(value);
      }
    }
  }

  else if (strcmp(command, "put") == 0) {
    struct entry_t* entry = _entry_create_from_args(arg1, arg2);
    if (entry == NULL) {
      printf("< Invalid args. Usage: put <key> <data>\n");
    } else {
      int op_id = rtree_put(rtree, entry);
      if (op_id < 0) {
        printf("< Operation failed!\n");
        entry_destroy(entry);
      } else {
        printf("< Operation queued with id %d.\n  (check result with command \"verify %d\")\n",
               op_id, op_id);
        entry_destroy(entry);
      }
    }
  }

  else if (strcmp(command, "getkeys") == 0) {
    char** keys = rtree_get_keys(rtree);
    if (keys == NULL) {
      printf("< Query failed!\n");
    } else {
      _print_keys(keys);
      tree_free_keys(keys);
    }
  }

  else if (strcmp(command, "verify") == 0) {
    if (arg1 == NULL || atoi(arg1) < 0) {
      printf("< Invalid args. Usage: verify <op_id>\n");
    } else {
      int op_result = rtree_verify(rtree, atoi(arg1));
      if (op_result < 0) {
        printf("< Operation failed!\n");
      } else if (op_result == 0) {
        printf("< Operation not yet executed!\n");
      } else {
        printf("< Operation done!\n");
      }
    }
  }

  else {
    printf("< Unknown command!\n");
  }
}

int main(int argc, char** argv) {

  signal(SIGPIPE, SIG_IGN);

  if (argc != 2) {
    errno = EINVAL;
    fprintf(stderr, "Invalid number of arguments\n");
    printf("Usage: ./tree_client <zookeeper-ip>:<zookeeper-port>\n");
    printf(" E.g.: ./tree_client 127.0.0.1:2181\n");
    return -1;
  }

  struct rtree_t* rtree = rtree_connect(argv[1]);
  if (rtree == NULL) {
    return -1;
  }

  while (1) {
    if (rtree_are_servers_connected() == 0) {
      printf("< Server not available.\n  Please, retry connecting with command \"retry\".\n");
    }

    char* input_str = _collect_input();
    if (input_str == NULL) {
      printf("< Failed to read the command. Please, try again.\n");
      continue;
    }

    if (strcmp(input_str, "quit") == 0) {
      free(input_str);
      break;
    }
    if (strcmp(input_str, "retry") == 0) {
      if (rtree_are_servers_connected()) {
        printf("< Server already connected\n");
      } else {
        servers_retry_connect(rtree);
      }
      free(input_str);
      continue;
    }
    if (rtree_are_servers_connected() == 0) {
      continue;
    }

    char* delimiter = " ";
    char* rest = NULL;
    char* command = strtok_r(input_str, delimiter, &rest);
    char* arg1 = strtok_r(rest, delimiter, &rest);
    char* arg2 = (strlen(rest) == 0) ? NULL : rest;

    _run_command(rtree, command, arg1, arg2);

    free(input_str);
  }

  printf("Quitting...\n");
  return rtree_disconnect(rtree);
}
