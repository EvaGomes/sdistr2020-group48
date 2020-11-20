/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "entry.h"
#include "logger-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tree-private.h"
#include "tree.h"

#include <stdlib.h>
#include <string.h>

#define SIZE_OF_INT sizeof(int)

int data_to_buffer(struct data_t* dataStruct, char** data_buf) {
  if (dataStruct == NULL || data_buf == NULL) {
    return -1;
  }

  DataMessage* msg = data_to_msg(dataStruct);
  if (msg == NULL) {
    return -1;
  }

  int buffer_size = data_message__get_packed_size(msg);
  uint8_t* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    logger_error_malloc_failed("data_to_buffer");
    return -1;
  }
  data_message__pack(msg, buffer);

  data_message__free_unpacked(msg, NULL);

  *data_buf = (char*) buffer; // bytes are bytes!
  return buffer_size;
}

struct data_t* buffer_to_data(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 0) {
    return NULL;
  }

  DataMessage* msg = data_message__unpack(NULL, buffer_size, (uint8_t*) buffer);
  struct data_t* data = msg_to_data(msg);
  data_message__free_unpacked(msg, NULL);
  
  return data;
}

int entry_to_buffer(struct entry_t* entry, char** entry_buf) {
  if (entry == NULL || entry_buf == NULL) {
    return -1;
  }

  EntryMessage* msg = entry_to_msg(entry);
  if (msg == NULL) {
    return -1;
  }

  int buffer_size = entry_message__get_packed_size(msg);
  uint8_t* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    logger_error_malloc_failed("entry_to_buffer");
    return -1;
  }
  entry_message__pack(msg, buffer);

  entry_message__free_unpacked(msg, NULL);

  *entry_buf = (char*) buffer;
  return buffer_size;
}

struct entry_t* buffer_to_entry(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 0) {
    return NULL;
  }

  EntryMessage* msg = entry_message__unpack(NULL, buffer_size, (uint8_t*) buffer);
  struct entry_t* entry = msg_to_entry(msg);
  entry_message__free_unpacked(msg, NULL);

  return entry;
}

/* Core recursive algorithm to collect the entry_to_buffer of each node of the tree
 *  (uses an pre-order traversal algorithm).
 */
int _collect_entry_bufs(struct tree_node_t* node, char** entries_bufs, int* len_entries_bufs,
                        int next_index) {
  if (node != NULL) {
    len_entries_bufs[next_index] = entry_to_buffer(node->entry, entries_bufs + next_index);
    next_index += 1;
    next_index = _collect_entry_bufs(node->left, entries_bufs, len_entries_bufs, next_index);
    next_index = _collect_entry_bufs(node->right, entries_bufs, len_entries_bufs, next_index);
  }
  return next_index;
}

int tree_to_buffer(struct tree_t* tree, char** tree_buf) {
  if (tree == NULL || tree_buf == NULL) {
    return -1;
  }

  char* entries_bufs[tree->size];
  int len_entries_bufs[tree->size];
  _collect_entry_bufs(tree->root, entries_bufs, len_entries_bufs, 0);

  int len_buffer = 0;
  for (int i = 0; i < tree->size; ++i) {
    len_buffer += SIZE_OF_INT;
    len_buffer += len_entries_bufs[i];
  }

  char* buffer = malloc(len_buffer);
  if (buffer == NULL) {
    logger_error_malloc_failed("tree_to_buffer");
    return -1;
  }
  int current_index = 0;
  for (int i = 0; i < tree->size; ++i) {
    memcpy(buffer + current_index, len_entries_bufs + i, SIZE_OF_INT);
    memcpy(buffer + current_index + SIZE_OF_INT, entries_bufs[i], len_entries_bufs[i]);

    free(entries_bufs[i]);
    current_index += SIZE_OF_INT + len_entries_bufs[i];
  }

  *tree_buf = buffer;
  return len_buffer;
}

struct tree_t* buffer_to_tree(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size <= 0) {
    return NULL;
  }

  struct tree_t* tree = tree_create();

  int current_index = 0;
  while (current_index < buffer_size) {
    int len_entry;
    memcpy(&len_entry, buffer + current_index, SIZE_OF_INT);
    struct entry_t* entry = buffer_to_entry(buffer + current_index + SIZE_OF_INT, len_entry);

    tree_put(tree, entry->key, entry->value);

    entry_destroy(entry);
    current_index += SIZE_OF_INT + len_entry;
  }
  return tree;
}

int message_to_buffer(struct message_t* message, char** message_buf) {
  if (message == NULL || message_buf == NULL) {
    return -1;
  }

  Message* msg = message->msg;
  if (msg == NULL) {
    return -1;
  }

  int buffer_size = message__get_packed_size(msg);
  uint8_t* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    logger_error_malloc_failed("message_to_buffer");
    return -1;
  }
  message__pack(msg, buffer);

  *message_buf = (char*) buffer;
  return buffer_size;
}

struct message_t* buffer_to_message(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 0) {
    return NULL;
  }

  Message* msg = message__unpack(NULL, buffer_size, (uint8_t*) buffer);
  struct message_t* message = malloc(sizeof(struct message_t*));
  if (message == NULL) {
    logger_error_malloc_failed("buffer_to_message");
    return NULL;
  }
  message->msg = msg;

  return message;
}
