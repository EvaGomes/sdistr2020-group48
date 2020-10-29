/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "entry.h"
#include "sdmessage.pb-c.h"
#include "tree-private.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_DATASIZE = sizeof(int);
const int SIZE_OF_INT = sizeof(int);

DataMessage _data_to_msg(struct data_t* dataStruct) {
  DataMessage msg;
  data_message__init(&msg);
  msg.data.len = dataStruct->datasize;
  msg.data.data = dataStruct->data;
  return msg;
}

int data_to_buffer(struct data_t* dataStruct, char** data_buf) {
  if (dataStruct == NULL || data_buf == NULL) {
    return -1;
  }

  DataMessage msg = _data_to_msg(dataStruct);
  int buffer_size = data_message__get_packed_size(&msg);
  uint8_t* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "\nERR: data_to_buffer: malloc failed\n");
    return -1;
  }
  data_message__pack(&msg, buffer);

  *data_buf = (char*) buffer; // bytes are bytes!
  return buffer_size;
}

struct data_t* _msg_to_data(DataMessage* data_msg) {
  int datasize = data_msg->data.len;
  if (datasize == 0) {
    return data_create(0);
  }
  void* data = malloc(datasize);
  if (data == NULL) {
    fprintf(stderr, "\nERR: _msg_to_data: malloc failed\n");
    return NULL;
  }
  memcpy(data, data_msg->data.data, datasize);
  return data_create2(datasize, data);
}

struct data_t* buffer_to_data(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 0) {
    return NULL;
  }

  DataMessage* msg = data_message__unpack(NULL, buffer_size, (uint8_t*) buffer);
  struct data_t* data = _msg_to_data(msg);

  data_message__free_unpacked(msg, NULL);
  return data;
}

int entry_to_buffer(struct entry_t* entry, char** entry_buf) {
  if (entry == NULL || entry_buf == NULL) {
    return -1;
  }

  EntryMessage msg;
  entry_message__init(&msg);
  msg.key = entry->key;
  if (entry->value != NULL) {
    DataMessage dataMsg = _data_to_msg(entry->value);
    msg.value = &dataMsg;
  }

  int buffer_size = entry_message__get_packed_size(&msg);
  uint8_t* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "\nERR: entry_to_buffer: malloc failed\n");
    return -1;
  }
  entry_message__pack(&msg, buffer);

  *entry_buf = (char*) buffer;
  return buffer_size;
}

struct entry_t* buffer_to_entry(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 0) {
    return NULL;
  }

  EntryMessage* msg = entry_message__unpack(NULL, buffer_size, (uint8_t*) buffer);
  char* key = (msg->key == NULL || strlen(msg->key) == 0) ? NULL : strdup(msg->key);
  struct data_t* value = (msg->value == NULL) ? NULL : _msg_to_data(msg->value);
  struct entry_t* entry = entry_create(key, value);

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
    fprintf(stderr, "\nERR: tree_to_buffer: malloc failed\n");
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
