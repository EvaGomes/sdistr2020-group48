/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "data.h"
#include "entry.h"

struct rtree_t;

struct rtree_t* rtree_connect(const char* address_port) {
  return 0; // TODO
}

int rtree_disconnect(struct rtree_t* rtree) {
  return 0; // TODO
}

int rtree_put(struct rtree_t* rtree, struct entry_t* entry) {
  return 0; // TODO
}

struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
  return 0; // TODO
}

int rtree_del(struct rtree_t* rtree, char* key) {
  return 0; // TODO
}

int rtree_size(struct rtree_t* rtree) {
  return 0; // TODO
}

int rtree_height(struct rtree_t* rtree) {
  return 0; // TODO
}

char** rtree_get_keys(struct rtree_t* rtree) {
  return 0; // TODO
}

void rtree_free_keys(char** keys) {
  // TODO
}
