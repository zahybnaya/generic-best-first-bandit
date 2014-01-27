#include "type.h"
#define USE_MINIMAX_REWARDS 0
void assignToType_sts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
  type_system *ts = (type_system *)void_ts;
  int i;
  
  if (fatherType == -1) { //no father means root node
    ((type_sts *)ts->types[0])->root = node;
    ((type_sts *)ts->types[0])->birth = ts->visits;
  } else {
    type_sts *t = (type_sts *)ts->types[fatherType];
    
    //The size of a type is the size of the subtree rooted at the node representing the type
    //which is updated at the expansion phase, so only need to take care of splitting types
    if (t->root->subtreeSize > threshold) {
      //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
      //is the ancestor of the current node to be inserted.
      //This ancestor will become the root of a new type which will be in the same index as the old type.
      treeNode *newFather = node;
      while (newFather->parent != t->root)
	newFather = newFather->parent;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	//skip empty children or this is the new father (will be taken care of later)
	if (!_DOM->isValidChild(t->root->rep, t->root->side, i) || t->root->children[i] == newFather){ 
	  continue;
	}
	ts->numTypes++;
	ts->types = (type**)realloc(ts->types, ts->numTypes * sizeof(type_sts *)); //alocate a new type
	ts->types[ts->numTypes - 1] = (type*)calloc(1, sizeof(type_sts));
	((type_sts *)ts->types[ts->numTypes - 1])->root = t->root->children[i];
	((type_sts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
	if (USE_MINIMAX_REWARDS){
		((type_sts *)ts->types[ts->numTypes - 1])->scoreSum = t->root->children[i]->minimaxScoreSum;
		((type_sts *)ts->types[ts->numTypes - 1])->mm_visits = t->root->children[i]->minimax_n;
	}else {
		((type_sts *)ts->types[ts->numTypes - 1])->scoreSum = t->root->children[i]->scoreSum;
	}
	if (policy == DELETE_VMAB)
	  ts->types[ts->numTypes - 1]->visits = 0;
	else
	  ts->types[ts->numTypes - 1]->visits = t->root->children[i]->n;
      }

      t->root = newFather;
      if (policy == DELETE_VMAB)
	      t->visits = 0;
      else
	      t->visits = newFather->n;

      t->birth = ts->visits;
      if (USE_MINIMAX_REWARDS){
	      t->scoreSum = newFather->minimaxScoreSum;
	      t->mm_visits = newFather->minimax_n;
      }else{ 
	      t->scoreSum = newFather->scoreSum;
      }
    }
  } 
}

treeNode *selectFromType_sts(void *void_t, double C) {
  type_sts *t = (type_sts *)void_t;
  treeNode *node = t->root;
  
  //travel down the tree using ucb
  while (_DOM->getGameStatus(node->rep) == INCOMPLETE && node->n > 0)
    node = node->children[selectMove(node, C)];
 
  return node;
}

void destroy_sts(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++)    
    free(ts->types[i]);
  
  free(ts->types);
}
