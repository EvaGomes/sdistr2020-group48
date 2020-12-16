/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "tree.h"
#include "data.h"
#include "entry-private.h"
#include "entry.h"
#include "logger-private.h"
#include "tree-private.h"

#include <stdlib.h>
#include <string.h>

#define SIZE_OF_TREE_T sizeof(struct tree_t)
#define SIZE_OF_TREE_NODE_T sizeof(struct tree_node_t)
#define SIZE_OF_CHAR_POINTER sizeof(char*)
#define SIZE_OF_NULL sizeof(NULL)

struct tree_t* tree_create() {
  struct tree_t* tree = malloc(SIZE_OF_TREE_T);
  if (tree == NULL) {
    logger_error_malloc_failed("tree_create");
    return NULL;
  }
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
  if (newNode == NULL) {
    logger_error_malloc_failed("_node_create");
    return NULL;
  }
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
  if (tree == NULL || key == NULL || value == NULL) {
    logger_error_invalid_args("tree_put");
    return -1;
  }

  char* copied_key = strdup(key);
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

/* Returns the pointer to the node with the given key, or NULL if it is not in the tree with the
 * given root. Expects the root to NOT be NULL.
 */
struct tree_node_t** _tree_node_pointer_get(struct tree_node_t** pointer_to_root, char* key) {
  struct tree_node_t** pointer_to_current_node = pointer_to_root;
  struct tree_node_t* current_node = *pointer_to_current_node;
  while (current_node != NULL) {
    int comparison = key_compare(key, current_node->entry->key);
    if (comparison == 0) {
      return pointer_to_current_node;
    } else {
      pointer_to_current_node = comparison < 0 ? &(current_node->left) : &(current_node->right);
      current_node = *pointer_to_current_node;
    }
  }
  return NULL; // node not found
}

struct data_t* tree_get(struct tree_t* tree, char* key) {
  if (tree == NULL) {
    logger_error_invalid_arg("tree_get", "tree", "NULL");
    return NULL;
  }
  if (tree->root == NULL) {
    return data_create(0);
  }
  struct tree_node_t** pointer_to_node = _tree_node_pointer_get(&(tree->root), key);
  if (pointer_to_node == NULL) {
    return data_create(0);
  }
  struct data_t* value = (*pointer_to_node)->entry->value;
  return data_dup(value);
}

/* Removes the node from the given pointer, replacing it with the appropriate descendant (if any)
 * so that the tree continues to be a Binary Search Tree. Also destroys the node.
 *  Expects the node in the pointer to be not NULL.
 */
void _tree_node_del(struct tree_node_t** pointer_to_node) {
  struct tree_node_t* node = *pointer_to_node;
  struct tree_node_t* replacement;

  if (node->left == NULL && node->right == NULL) {
    replacement = NULL; // no children: simply remove node
  } else if (node->left == NULL) {
    replacement = node->right; // only child: use it to replace node
  } else if (node->right == NULL) {
    replacement = node->left; // only child: use it to replace node
  } else {
    // two children: replace the node to delete with the successor (node from the right subtree)
    // that has the lowest key

    // find the successor that has the lowest key
    struct tree_node_t** pointer_to_successor_with_lowest_key = &(node->right);
    struct tree_node_t* successor_with_lowest_key = *pointer_to_successor_with_lowest_key;
    while (successor_with_lowest_key->left != NULL) {
      pointer_to_successor_with_lowest_key = &(successor_with_lowest_key->left);
      successor_with_lowest_key = *pointer_to_successor_with_lowest_key;
    }

    // such successor has, at most, 1 child and it would be on the right side.
    // if existent, replace sucessor with the child and remove child from successor
    *pointer_to_successor_with_lowest_key = successor_with_lowest_key->right;

    // do replace
    replacement = successor_with_lowest_key;
    replacement->left = node->left;
    replacement->right = node->right;
  }

  *pointer_to_node = replacement;
  entry_destroy(node->entry);
  free(node);
}

/* Recursive algorithm to compute the height of the tree starting at the given node. */
int _tree_compute_height(struct tree_node_t* node) {
  if (node == NULL) {
    return -1;
  }
  int height_of_left_tree = _tree_compute_height(node->left);
  int height_of_right_tree = _tree_compute_height(node->right);
  return 1 + _int_max(height_of_left_tree, height_of_right_tree);
}

int tree_del(struct tree_t* tree, char* key) {
  if (tree == NULL || tree->root == NULL) {
    logger_error_invalid_args("tree_del");
    return -1;
  }
  if (tree->root != NULL) {
    struct tree_node_t** pointer_to_node = _tree_node_pointer_get(&(tree->root), key);
    if (pointer_to_node != NULL) {
      _tree_node_del(pointer_to_node);
      tree->size -= 1;
      tree->height = _tree_compute_height(tree->root);
    }
  }
  return 0;
}

int tree_size(struct tree_t* tree) {
  return tree == NULL ? -1 : tree->size;
}

int tree_height(struct tree_t* tree) {
  return tree == NULL ? -1 : tree->height;
}

/* Core recursive algorithm for tree_get_keys (uses an in-order traversal algorithm). */
int _collect_keys(struct tree_node_t* node, char** keys, int next_index) {
  if (node != NULL) {
    next_index = _collect_keys(node->left, keys, next_index);
    keys[next_index] = strdup(node->entry->key);
    next_index += 1;
    next_index = _collect_keys(node->right, keys, next_index);
  }
  return next_index;
}

char** tree_get_keys(struct tree_t* tree) {
  if (tree == NULL) {
    logger_error_invalid_arg("tree_del", "tree", "NULL");
    return NULL;
  }
  char** keys = malloc(SIZE_OF_CHAR_POINTER * tree->size + SIZE_OF_NULL);
  if (keys == NULL) {
    logger_error_malloc_failed("tree_get_keys");
    return NULL;
  }
  int last_index = _collect_keys(tree->root, keys, 0);
  keys[last_index] = NULL;
  return keys;
}

void tree_free_keys(char** keys) {
  if (keys != NULL) {
    for (int i = 0; keys[i] != NULL; ++i) {
      free(keys[i]);
    }
    free(keys);
  }
}
