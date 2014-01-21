#include "type.h"

static int selectMove_vts(treeNode* node, double C) {
  int i;
  double qhat;
  double score;
  int numBestMoves = 0;
  double bestScore;
  int bestMoves[_DOM->getNumOfChildren()];

  // The multiplier is used to set the sign of the exploration bonus term (should be negative
  // for the min player and positive for the max player) i.e. so that we correctly compute
  // an upper confidence bound for Max and a lower confidence bound for Min
  double multiplier = (node->side == max) ? 1 : -1;

  for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all children
    if (!_DOM->isValidChild(node->rep, node->side, i)) // if the i^th move is illegal, skip it
      continue;

    // If the i^th child has never been visited before, select it first, before any other children are revisited
    if (node->children[i]->n == 0)
      return i;

    // Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
    // children have been visited at least once)
    qhat = node->children[i]->scoreSum / (double)node->children[i]->n;  // exploitation component (this is the average utility)
    score = qhat + (multiplier * C) * sqrt(log(node->n) / (double)node->children[i]->n); // add exploration component

    // Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
    // is min's turn means we can always just take the maximum
    score = (node->side == min) ? -score : score;
   
    // If this is either the first child, or the best scoring child, store it
    if ((numBestMoves == 0) || (score > bestScore)) {
      bestMoves[0] = i;
      bestScore = score;
      numBestMoves = 1;
    }
    else if (score == bestScore) // if this child ties with the best scoring child, store it
      bestMoves[numBestMoves++] = i;
  }

  return bestMoves[0];
}

treeNode *selectFromType_vts(void *void_t, double C) {
  type_sts *t = (type_sts *)void_t;
  treeNode *node = t->root;
  
  //travel down the tree using ucb
  while (_DOM->getGameStatus(node->rep) == INCOMPLETE && node->n > 0)
    node = node->children[selectMove_vts(node, C)];
 
  return node;
}

//TODO
void assignToType_vts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
  type_system *ts = (type_system *)void_ts;
  int i;
  
  if (fatherType == -1) { //no father means root node
    ((type_vts *)ts->types[0])->root = node;
    ((type_vts *)ts->types[0])->birth = ts->visits;
    node->typeDefiner = true;
  } else {
    type_vts *t = (type_vts *)ts->types[fatherType];
    
    //The size of a type is updated during backpropagation and value split so only need to take care of splitting types
    if (t->size > threshold) {
      //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
      //is the ancestor of the current node to be inserted.
      //This ancestor will become the root of a new type which will be in the same index as the old type.
      treeNode *newFather = node;
      while (newFather->parent != t->root)
	newFather = newFather->parent;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	if (!_DOM->isValidChild(t->root->rep, t->root->side, i) || t->root->children[i] == newFather) //skip empty children or this is the new father (will be taken care of later)
	  continue;
	
	ts->numTypes++;
	ts->types = realloc(ts->types, ts->numTypes * sizeof(type_vts *)); //alocate a new type
	
	ts->types[ts->numTypes - 1] = calloc(1, sizeof(type_vts));
	((type_vts *)ts->types[ts->numTypes - 1])->root = t->root->children[i];
	((type_vts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
	
	if (policy == DELETE_VMAB)
	  ts->types[ts->numTypes - 1]->visits = 0;
	else
	  ts->types[ts->numTypes - 1]->visits = t->root->children[i]->n;
      }
      
      t->root = node;
      if (policy == DELETE_VMAB)
	t->visits = 0;
      else
	t->visits = node->n;
      
      t->birth = ts->visits;
    }
  } 
}

void destroy_vts(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++) {
    free(((type_vts *)ts->types[i])->childrenTypedNodes);
    free(ts->types[i]);
  }
  
  free(ts->types);
}

void typeSignificance(type_system *ts, type_vts *type, treeNode **path) {
  int pathLength = path[0]->depth - type->root->depth;
  int i, j;
  double nodeSd; //standard deviation of current node
  double nodeMean; //mean of the current node;
  treeNode *node; //current node
  double z; //the current z-test value
  
  //mean of the type
  double typeMean = type->scoreSum / (double)type->visits;
  
  treeNode *childOfTypeRoot = path[pathLength - 1];
  
  for (i = pathLength - 1; i >= 0; i--) {
    
    //Split based on value only when there were more than X rollouts from this subtree.
    if (node->n < 30) //TODO param this?
      break;
    
    node = path[i];
    nodeSd = sqrt(node->deviationSum / (double)node->n);
    nodeMean = node->scoreSum / (double)node->n;
    z = (nodeMean - typeMean) / (nodeSd / sqrt(node->n));
    
    if (z > 2.33) { //TODO: param this?
      ts->numTypes++;
      ts->types = realloc(ts->types, ts->numTypes * sizeof(type_vts *)); //alocate a new type
      
      //New Type
      ts->types[ts->numTypes - 1] = calloc(1, sizeof(type_vts));
      ((type_vts *)ts->types[ts->numTypes - 1])->root = node;
      node->typeDefiner = true;
      ((type_vts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
      ((type_vts *)ts->types[ts->numTypes - 1])->childrenTypedNodes = calloc(_DOM->getNumOfChildren(), sizeof(int));
      ((type_vts *)ts->types[ts->numTypes - 1])->scoreSum = node->scoreSum;
      ((type_vts *)ts->types[ts->numTypes - 1])->deviationSum = node->deviationSum;
      ((type_vts *)ts->types[ts->numTypes - 1])->visits = node->n;
      ((type_vts *)ts->types[ts->numTypes - 1])->size = node->subtreeSize;
      
      //Old Type
      type->visits = type->visits - node->n;
      type->scoreSum = type->scoreSum - node->scoreSum;
      type->deviationSum = type->deviationSum - node->deviationSum;
      type->size = type->size - node->subtreeSize;
      for (j = 0; j < _DOM->getNumOfChildren(); j++)
	if (type->root->children[j] == childOfTypeRoot)
	  type->childrenTypedNodes[j] = type->childrenTypedNodes[j] + node->subtreeSize;

      break;
    }
  }
}