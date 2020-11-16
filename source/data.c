/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "data-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_DATA_T = sizeof(struct data_t);

struct data_t* data_create(int size) {
  if (size < 0) {
    fprintf(stderr, "\nERR: data_create: invalid size %d\n", size);
    return NULL;
  }
  if (size == 0) {
    return data_create2(0, NULL);
  }
  void* data = malloc(size);
  if (data == NULL) {
    fprintf(stderr, "\nERR: data_create: malloc failed\n");
    return NULL;
  }
  return data_create2(size, data);
}

int _areParamsInvalid(int size, void* data) {
  return (size < 0) || (size != 0 && data == NULL);
}

struct data_t* data_create2(int size, void* data) {
  if (_areParamsInvalid(size, data)) {
    return NULL;
  }
  struct data_t* dataStruct = malloc(SIZE_OF_DATA_T);
  if (dataStruct == NULL) {
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

void* copy(void* something, int size) {
  if (something == NULL) {
    return NULL;
  }
  void* copy = malloc(size);
  if (copy == NULL) {
    fprintf(stderr, "\nERR: copy: malloc failed\n");
    return NULL;
  }
  memcpy(copy, something, size);
  return copy;
}

struct data_t* data_dup(struct data_t* dataStruct) {
  if (dataStruct == NULL || _areParamsInvalid(dataStruct->datasize, dataStruct->data)) {
    return NULL;
  }
  int datasize = dataStruct->datasize;
  void* copied_data = copy(dataStruct->data, datasize);
  return data_create2(datasize, copied_data);
}

void data_replace(struct data_t* dataStruct, int new_size, void* new_data) {
  if (dataStruct != NULL && !_areParamsInvalid(new_size, new_data)) {
    free(dataStruct->data);
    dataStruct->datasize = new_size;
    dataStruct->data = new_data;
  }
}
