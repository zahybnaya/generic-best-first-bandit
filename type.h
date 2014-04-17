#ifndef TYPE_H
#define TYPE_H
#include "common.h"
#include "domain.h"

extern DOM* _DOM;

#define TYPE_FROM_ROOT 0
#define CHOOSE_TYPE 1
#define UCB 0
#define E_GREEDY 1
#define PROB_AND_AVG 2
#define PROB_AND_AVG_AND_EXPLORE 3
#define E_GREEDY_PATH_PROB 4
#define GREEDY_EPSILON 0.25

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
  double pathProb; //In MDP, the probability of getting to this state from the root.
  int typedN; // == n - visits in subtrees which are not of the same type as this node
  double typedScoreSum; // == scoreSum - rollouts from subtrees which are not of the same type as this node
} treeNode;

typedef void (*backprop_func)(void *ts, treeNode *node, double rollout, int generated, int typeId, int threshold);

typedef struct {
  int name;
  treeNode **types;
  int *birthdays;
  int numTypes;
  int visits;
  backprop_func backprop;
} type_system;

//Type system (type.c)
void *init_type_system(int t);
int selectMove(treeNode* node, double C);
void destroy_ts(void *void_ts);

//STS functions
void backprop_sts(void *void_ts, treeNode *node, double rollout, int generated, int typeId, int threshold);

//CITS functions
void backprop_cits(void *void_ts, treeNode *node, double rollout, int generated, int typeId, int threshold);

//BFB for sailing (bfb.c)
double actionCostFindMove(treeNode *node);

#endif /* end of include guard: TYPE_H */
