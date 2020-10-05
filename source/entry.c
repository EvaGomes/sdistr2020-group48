/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "entry.h"
#include "data.h"

struct entry_t* entry_create(char* key, struct data_t* data) {
  return 0; // TODO
}

void entry_initialize(struct entry_t* entry) {
  // TODO
}

void entry_destroy(struct entry_t* entry) {
  // TODO
}

struct entry_t* entry_dup(struct entry_t* entry) {
  return 0; // TODO
}

void entry_replace(struct entry_t* entry, char* new_key,
                   struct data_t* new_value) {
  // TODO
}

int entry_compare(struct entry_t* entry1, struct entry_t* entry2) {
  return 0; // TODO
}
