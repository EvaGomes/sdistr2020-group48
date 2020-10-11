/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "entry.h"
#include "tree-private.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_DATASIZE = sizeof(int);
const int SIZE_OF_INT = sizeof(int);

int data_to_buffer(struct data_t* dataStruct, char** data_buf) {
  if (dataStruct == NULL || data_buf == NULL) {
    return -1;
  }

  int* pointer_datasize = &(dataStruct->datasize);
  void* pointer_data = dataStruct->data;

  int len_datasize = SIZE_OF_DATASIZE;
  int len_data = dataStruct->datasize;

  int index_datasize = 0;
  int index_data = len_datasize;

  int len_buffer = len_datasize + len_data;
  char* buffer = malloc(len_buffer);
  memcpy(buffer + index_datasize, pointer_datasize, len_datasize);
  memcpy(buffer + index_data, pointer_data, len_data);

  *data_buf = buffer;
  return len_buffer;
}

struct data_t* buffer_to_data(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size <= 0) {
    return NULL;
  }

  int datasize;
  int len_datasize = SIZE_OF_DATASIZE;
  int index_datasize = 0;
  memcpy(&datasize, buffer + index_datasize, len_datasize);

  void* data = malloc(datasize);
  if (data == NULL) {
    fprintf(stderr, "\nERR: buffer_to_data: malloc failed\n");
    return NULL;
  }
  int len_data = datasize;
  int index_data = index_datasize + len_datasize;
  memcpy(data, buffer + index_data, len_data);

  return data_create2(datasize, data);
}

int entry_to_buffer(struct entry_t* entry, char** entry_buf) {
  if (entry == NULL || entry->key == NULL || entry->value == NULL || entry_buf == NULL) {
    return -1;
  }

  char* value;

  int len_len_key = sizeof(int);
  int len_key = strlen(entry->key) + 1;
  int len_value = data_to_buffer(entry->value, &value);

  int index_len_key = 0;
  int index_key = len_len_key;
  int index_value = len_len_key + len_key;

  int buffer_size = len_len_key + len_key + len_value;
  char* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "\nERR: entry_to_buffer: malloc failed\n");
    return -1;
  }
  memcpy(buffer + index_len_key, &len_key, len_len_key);
  memcpy(buffer + index_key, entry->key, len_key);
  memcpy(buffer + index_value, value, len_value);

  free(value);

  *entry_buf = buffer;
  return buffer_size;
}

struct entry_t* buffer_to_entry(char* buffer, int buffer_size) {
  if (buffer == NULL || buffer_size <= 0) {
    return NULL;
  }

  int len_key;
  int len_len_key = sizeof(len_key);
  int index_len_key = 0;
  memcpy(&len_key, buffer + index_len_key, len_len_key);

  char* key = malloc(len_key);
  if (key == NULL) {
    fprintf(stderr, "\nERR: buffer_to_entry: malloc failed\n");
    return NULL;
  }
  int index_key = index_len_key + len_len_key;
  memcpy(key, buffer + index_key, len_key);

  int index_value = index_key + len_key;
  int len_value = buffer_size - index_value;
  struct data_t* value = buffer_to_data(buffer + index_value, len_value);

  return entry_create(key, value);
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
