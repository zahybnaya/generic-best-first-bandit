#include "type.h"
#define USE_MINIMAX_REWARDS 1

/**
 * Add types to ts which are decendents of type t and result from move i 
 */
static void addType( type_system* ts, type_sts* t,treeNode* splitCandidate, treeNode* newFather,int policy){
			  if (t->root->side != splitCandidate->side || t->root == splitCandidate) {
					  int i;  
					  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
							  if (!_DOM->isValidChild(splitCandidate->rep, splitCandidate->side, i) ||splitCandidate->children[i] == newFather || 
											  splitCandidate->children[i]==NULL){ 
									  continue;
							  }
							  addType(ts,t,splitCandidate->children[i],newFather,policy);
					  }
					  return;
			  }
			  ts->numTypes++;
			  ts->types = (type**)realloc(ts->types, ts->numTypes * sizeof(type_sts *)); //alocate a new type
			  ts->types[ts->numTypes - 1] = (type*)calloc(1, sizeof(type_sts));
			  ((type_sts *)ts->types[ts->numTypes - 1])->root = splitCandidate;
			  ((type_sts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
			  ((type_sts *)ts->types[ts->numTypes - 1])->minmax = splitCandidate->minmax;
			  if (USE_MINIMAX_REWARDS){
					  ((type_sts *)ts->types[ts->numTypes - 1])->scoreSum = splitCandidate->minimaxScoreSum;
					  ((type_sts *)ts->types[ts->numTypes - 1])->visits = splitCandidate->minimax_n;
			  }else {
					  ((type_sts *)ts->types[ts->numTypes - 1])->scoreSum = splitCandidate->scoreSum;
			  }
			  if (policy == DELETE_VMAB)
					  ts->types[ts->numTypes - 1]->visits = 0;
			  else
					  ts->types[ts->numTypes - 1]->visits = splitCandidate->n;
			  assert(splitCandidate->side == t->root->side);
}

void assignToType_sts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
  type_system *ts = (type_system *)void_ts;
  if (fatherType == -1) { //no father means root node
    ((type_sts *)ts->types[0])->root = node;
    ((type_sts *)ts->types[0])->birth = ts->visits;
  } else {
	  type_sts *t = (type_sts *)ts->types[fatherType];
	  if (t->root->subtreeSize > threshold) {
		  //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
		  //is the ancestor of the current node to be inserted.
		  //This ancestor will become the root of a new type which will be in the same index as the old type.
		  treeNode *newFather = t->root; //On very rare cases, there are no childrent with different side all the way down
		  int originalTypeSide = t->root->side;
		  treeNode* searchFather = node;
		  while (searchFather!= t->root){
		  	if (searchFather->side == originalTypeSide){
					newFather = searchFather;
			}
				searchFather=searchFather->parent;
		  }
		  assert(newFather->side == t->root->side);
          addType(ts,t,t->root,newFather,policy);
		  t->root = newFather;
		  if (policy == DELETE_VMAB)
				  t->visits = 0;
		  else
				  t->visits = newFather->n;
		  t->birth = ts->visits;
		  t->minmax = newFather->minmax;
		  if (USE_MINIMAX_REWARDS){
				  t->scoreSum = newFather->minimaxScoreSum;
				  t->visits = newFather->minimax_n;
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
    node = node->children[selectMove(node, C,false)];
 
  return node;
}

void destroy_sts(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++)    
    free(ts->types[i]);
  
  free(ts->types);
}
