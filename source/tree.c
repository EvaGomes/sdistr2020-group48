/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "tree.h"
#include "data.h"
#include "entry-private.h"
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

/* Creates a node for the given key and value (with no children). */
struct tree_node_t* _node_create(char* key, struct data_t* value) {
  struct tree_node_t* newNode = malloc(SIZE_OF_TREE_NODE_T);
  newNode->entry = entry_create(key, value);
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

/* Returns the maximum between two ints. */
int _int_max(int n1, int n2) {
  return n1 > n2 ? n1 : n2;
}

/* Core iterative algorithm of tree_put.
 *  Expects the tree to have a root, and the key and value to be ready to store.
 */
void _tree_put(struct tree_t* tree, char* copied_key, struct data_t* copied_value) {
  struct tree_node_t* current_tree_node = tree->root;
  int current_depth = 0;

  while (1) {
    int comparison = key_compare(copied_key, current_tree_node->entry->key);

    if (comparison == 0) {
      entry_destroy(current_tree_node->entry);
      current_tree_node->entry = entry_create(copied_key, copied_value);
      return; // replaced current_tree_node's entry (no changes in size nor height)
    }

    struct tree_node_t** pointer_to_next_node =
        (comparison < 0 ? &(current_tree_node->left) : &(current_tree_node->right));

    if (*pointer_to_next_node == NULL) {
      *pointer_to_next_node = _node_create(copied_key, copied_value);
      tree->size += 1;
      tree->height = _int_max(current_depth + 1, tree->height);
      return; // inserted new node below the current_tree_node
    }

    // update vars for next iteration of the walk
    current_tree_node = *pointer_to_next_node;
    current_depth += 1;
  }
}

int tree_put(struct tree_t* tree, char* key, struct data_t* value) {
  if (tree == NULL) {
    return -1;
  }

  char* copied_key = key == NULL ? NULL : strdup(key);
  struct data_t* copied_value = data_dup(value);

  if (tree->root == NULL) {
    // insert root node
    tree->root = _node_create(copied_key, copied_value);
    tree->size = 1;
    tree->height = 0;
  } else {
    // insert node in the correct place
    _tree_put(tree, copied_key, copied_value);
  }
  return 0;
}

struct data_t* tree_get(struct tree_t* tree, char* key) {
  if (tree == NULL || tree->root == NULL) {
    return NULL;
  }

  struct tree_node_t* current_tree_node = tree->root;
  while (current_tree_node != NULL) {
    int comparison = key_compare(key, current_tree_node->entry->key);
    if (comparison == 0) {
      return data_dup(current_tree_node->entry->value);
    } else {
      current_tree_node = comparison < 0 ? current_tree_node->left : current_tree_node->right;
    }
  }

  return NULL; // not found
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
