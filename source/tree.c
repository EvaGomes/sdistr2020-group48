/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "tree.h"
#include "data.h"
#include "entry.h"
#include "tree-private.h"
#include <stdlib.h>
#include <string.h>

const int SIZE_OF_TREE_T = sizeof(struct tree_t);
const int SIZE_OF_TREE_NODE_T = sizeof(struct tree_node_t);

struct tree_t* tree_create() {
  struct tree_t* tree = malloc(SIZE_OF_TREE_T);
  tree->root = NULL;
  tree->size = 0;
  tree->height = -1;
  return tree;
}

/* Destroys, recursively, a node of the tree and its children. */
void _tree_node_destroy(struct tree_node_t* node) {
  if (node != NULL) {
    entry_destroy(node->entry);
    _tree_node_destroy(node->left);
    _tree_node_destroy(node->right);
    free(node);
  }
}

void tree_destroy(struct tree_t* tree) {
  if (tree != NULL) {
    _tree_node_destroy(tree->root);
    free(tree);
  }
}

int tree_put(struct tree_t* tree, char* key, struct data_t* value) {
  return 0; // TODO
}

struct data_t* tree_get(struct tree_t* tree, char* key) {
  return 0; // TODO
}

int tree_del(struct tree_t* tree, char* key) {
  return 0; // TODO
}

int tree_size(struct tree_t* tree) {
  return tree == NULL ? -1 : tree->size;
}

int tree_height(struct tree_t* tree) {
  return tree == NULL ? -1 : tree->height;
}

char** tree_get_keys(struct tree_t* tree) {
  return 0; // TODO
}

void tree_free_keys(char** keys) {
  // TODO
}
