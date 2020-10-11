/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_DATA_T = sizeof(struct data_t);

struct data_t* data_create(int size) {
  if (size <= 0) {
    return NULL;
  }
  void* data = malloc(size);
  if (data == NULL) {
    fprintf(stderr, "\nERR: data_create: malloc failed\n");
    return NULL;
  }
  return data_create2(size, data);
}

struct data_t* data_create2(int size, void* data) {
  if (size <= 0 || data == NULL) {
    return NULL;
  }
  struct data_t* dataStruct = malloc(SIZE_OF_DATA_T);
  if (data == NULL) {
    fprintf(stderr, "\nERR: data_create2: malloc failed\n");
    return NULL;
  }
  dataStruct->datasize = size;
  dataStruct->data = data;
  return dataStruct;
}

void data_destroy(struct data_t* dataStruct) {
  if (dataStruct != NULL) {
    free(dataStruct->data);
    free(dataStruct);
  }
}

struct data_t* data_dup(struct data_t* dataStruct) {
  if (dataStruct == NULL || dataStruct->datasize <= 0 || dataStruct->data == NULL) {
    return NULL;
  }
  int size = dataStruct->datasize;
  void* copied_data = malloc(size);
  if (copied_data == NULL) {
    fprintf(stderr, "\nERR: data_dup: malloc failed\n");
    return NULL;
  }
  memcpy(copied_data, dataStruct->data, size);
  return data_create2(size, copied_data);
}

void data_replace(struct data_t* dataStruct, int new_size, void* new_data) {
  if (dataStruct != NULL) {
    free(dataStruct->data);
    dataStruct->datasize = new_size;
    dataStruct->data = new_data;
  }
}
