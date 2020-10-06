/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "entry.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_DATASIZE = sizeof(int);

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
  int len_data = datasize;
  int index_data = index_datasize + len_datasize;
  memcpy(data, buffer + index_data, len_data);

  return data_create2(datasize, data);
}

int entry_to_buffer(struct entry_t* entry, char** entry_buf) {
  return 0; // TODO
}

struct entry_t* buffer_to_entry(char* entry_buf, int entry_buf_size) {
  return 0; // TODO
}

int tree_to_buffer(struct tree_t* tree, char** tree_buf) {
  return 0; // TODO
}

struct entry_t* buffer_to_tree(char* tree_buf, int tree_buf_size) {
  return 0; // TODO
}
