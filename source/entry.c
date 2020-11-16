/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "entry.h"
#include "data.h"
#include "entry-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_OF_ENTRY_T sizeof(struct entry_t)

struct entry_t* entry_create(char* key, struct data_t* data) {
  struct entry_t* entry = malloc(SIZE_OF_ENTRY_T);
  if (entry == NULL) {
    fprintf(stderr, "\nERR: entry_create: malloc failed\n");
    return NULL;
  }
  entry->key = key;
  entry->value = data;
  return entry;
}

void entry_initialize(struct entry_t* entry) {
  if (entry != NULL) {
    entry->key = NULL;
    entry->value = NULL;
  }
}

void entry_destroy(struct entry_t* entry) {
  if (entry != NULL) {
    free(entry->key);
    data_destroy(entry->value);
    free(entry);
  }
}

struct entry_t* entry_dup(struct entry_t* entry) {
  if (entry == NULL) {
    return NULL;
  }
  char* copied_key = entry->key == NULL ? NULL : strdup(entry->key);
  struct data_t* copied_value = data_dup(entry->value);
  return entry_create(copied_key, copied_value);
}

void entry_replace(struct entry_t* entry, char* new_key, struct data_t* new_value) {
  if (entry != NULL) {
    free(entry->key);
    data_destroy(entry->value);
    entry->key = new_key;
    entry->value = new_value;
  }
}

// defined at entry-private.h
int key_compare(char* key1, char* key2) {
  if (key1 == key2) {
    return 0;
  }
  if (key1 == NULL) {
    return -1;
  }
  if (key2 == NULL) {
    return +1;
  }
  int comparison = strcmp(key1, key2);
  return comparison < 0 ? -1 : (comparison == 0 ? 0 : +1);
}

int entry_compare(struct entry_t* entry1, struct entry_t* entry2) {
  if (entry1 == entry2) {
    return 0;
  }
  if (entry1 == NULL) {
    return -1;
  }
  if (entry2 == NULL) {
    return +1;
  }
  return key_compare(entry1->key, entry2->key);
}
