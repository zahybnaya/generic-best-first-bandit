#ifndef TYPE_H
#define TYPE_H
#include "common.h"
#include "domain.h"

extern DOM* _DOM;

/* This is a node in the BFB tree. */
typedef struct node {
  double scoreSum; //sum of all rollouts gone through this node
  int n; // tracks the visit count
  int id; // used for graph visualization purposes
  int depth;
  int subtreeSize; //size of subtree rooted at this node. equals one when this node has no children.
  int type; //true if defining a type (a switch point to uct)
  rep_t rep; // generic representation of the state
  int side; // side on move at this board position
  struct node *parent; //the parent node
  struct node** children; /* pointers to the children of this node -- note that index 0 remains
					unused (which is reserved for the store), so we have consistent move
					indexing/numbering */
} treeNode;

typedef struct {
  int name;
  treeNode **types;
  int *birthdays;
  int numTypes;
  int visits;
} type_system;

//Type system (type.c)
void *init_type_system(int t);
int selectMove(treeNode* node, double C);
void destroy_ts(void *void_ts);

#endif /* end of include guard: TYPE_H */
