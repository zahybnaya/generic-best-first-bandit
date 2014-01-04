#include "common.h"
#include "domain.h"

#define ORACLE_DEPTH 12
#define ORACLE_PATH "MMValues.txt"
//heuristic used by the oracle minimax
#define ORACLE_H _DOM->hFunctions.h2
//heuristic used by assign to type for mystery nodes
#define ORACLE_MISSING_H(x,y) _DOM->hFunctions.h2(x, y, 1)

extern DOM* _DOM;

//TODO extract and merge with uct.c
/* This is a node in the UCT tree. */
typedef struct node {
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this node
  int n; // tracks the visit count
  int id; // used for graph visualization purposes
  int depth;
  int subtreeSize;
  rep_t rep; // generic representation of the state
  int side; // side on move at this board position
  struct node *parent; //the parent node
  struct node** children; /* pointers to the children of this node -- note that index 0 remains
					unused (which is reserved for the store), so we have consistent move
					indexing/numbering */
} treeNode;

typedef struct {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  
  //Type stats
  treeNode **openList; //tree nodes which havn't been fully expanded and are of the same type
  int capacity; //Maximum current capacity of the open list.
  int tail;  //The first empty index in the open list.
  int empty; //If a node was extracted from the open list than this will be its index until filled again, else -1.
  int size; //Current number of nodes in the type
} type;

typedef void (*assignToType_func)(void *void_ts, treeNode *node, int fatherType, int threshold);
typedef int (*selectType_func)(void *void_ts, double C, int visits, int side);
typedef treeNode *(*selectFromType_func)(type *t, double C);
typedef void (*furtherInit_func)(void *void_ts, rep_t rep, int side);

typedef struct {
  int name;
  type **types;
  int numTypes;
  void *extra; //extra data depending on the type system
  
  furtherInit_func furtherInit;
  assignToType_func assignToType;
  selectType_func selectType;
  selectFromType_func selectFromType;
} type_system;

enum {MM_ORACLE = 1, STS = 2};