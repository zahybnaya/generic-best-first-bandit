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
  double deviationSum; //stores the sum of the deviation of rollout gone through this node
  int n; // tracks the visit count
  int id; // used for graph visualization purposes
  int depth;
  int subtreeSize; //size of subtree rooted at this node. equals one when this node has no children.
  rep_t rep; // generic representation of the state
  int side; // side on move at this board position
  int typeDefiner; //is this node difining a type? (For use in VTS)
  struct node *parent; //the parent node
  struct node** children; /* pointers to the children of this node -- note that index 0 remains
					unused (which is reserved for the store), so we have consistent move
					indexing/numbering */
} treeNode;

typedef struct {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  int birth; //the iteration this type appeared
} type;

typedef struct {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  int birth; //the iteration this type appeared
  
  //Type stats
  treeNode **openList; //tree nodes which havn't been fully expanded and are of the same type
  int capacity; //Maximum current capacity of the open list.
  int tail;  //The first empty index in the open list.
  int empty; //If a node was extracted from the open list than this will be its index until filled again, else -1.
  int size; //Current number of nodes in the type
} type_mmOracle;

typedef struct {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  int birth; //the iteration this type appeared
  
  //Type stats: subtree size is updated within the node
  treeNode *root; //the root of the subtree that this type represents
} type_sts;

typedef struct {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  int birth; //the iteration this type appeared
  
  //Type stats:
  treeNode *root; //the root of the subtree that this type represents
  double deviationSum; //stores the sum of the deviation of rollouts gone through this node
  int size; //the number of nodes within this type
  int *childrenTypedNodes; //the number of nodes in each subtree of the children of the root node that are a part of a type already. 
} type_vts;

typedef void (*assignToType_func)(void *void_ts, treeNode *node, int fatherType, int threshold, int policy);
typedef treeNode *(*selectFromType_func)(void *void_t, double C);
typedef void (*destroy_func)(void *void_ts);

typedef struct {
  int name;
  type **types;
  int numTypes;
  void *extra; //extra data depending on the type system
  int visits;
  
  assignToType_func assignToType;
  selectFromType_func selectFromType;
  destroy_func destroy;
} type_system;

//Type system (type.c)
void *init_type_system(int t, rep_t rep, int side);
void destroyTypeSystem(void *void_ts);

//VTS (vts.c)
void typeSignificance(type_system *ts, type_vts *type, treeNode **path);
void assignToType_vts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy);
treeNode *selectFromType_vts(void *void_t, double C);
void destroy_vts(void *void_ts);

//STS (sts.c)
void assignToType_sts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy);
treeNode *selectFromType_sts(void *void_t, double C);
void destroy_sts(void *void_ts);

//OracleTS (oracleTypeSystem.c)
double storeMinimax(int fd, rep_t rep, int searchDepth, int depth, int side, heuristics_t heuristic, int budget);
void assignToType_mmOracle(void *void_ts, treeNode *node, int fatherType, int threshold, int policy);
treeNode *selectFromType_mmOracle(void *void_t, double C);
void destroy_mmOracle(void *void_ts);