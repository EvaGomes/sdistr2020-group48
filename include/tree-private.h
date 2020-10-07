/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */
#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"
#include "tree.h"

/* The nodes of a tree_t. */
struct tree_node_t {
  /* The key and value of this node. It must NOT be NULL). */
  struct entry_t* entry;
  /* The left child-node (its key must be lower than this node's key). It may be NULL. */
  struct tree_node_t* left;
  /* The right child-node (its key must be greater than this node's key). It may be NULL. */
  struct tree_node_t* right;
};

/* A binary search tree with nodes of type tree_node_t. */
struct tree_t {
  /** The root-node of the tree. It may be NULL. */
  struct tree_node_t* root;
  /** The size of this tree (i.e. the number of nodes). */
  int size;
  /** The height of this tree (i.e., the number of edges between the root and the deepest node). */
  int height;
};

#endif
