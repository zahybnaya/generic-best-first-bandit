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
    if (!_DOM->isValidChild(node->rep, node->side, i) || node->children[i]->typeDefiner == true) // if the i^th move is illegal, skip it or not of this type
      continue;
    
    // If the i^th child has never been visited before, select it first, before any other children are revisited
    if (node->children[i]->typedN == 0)
      return i;

    // Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
    // children have been visited at least once)
    qhat = node->children[i]->typedScoreSum / (double)node->children[i]->typedN;  // exploitation component (this is the average utility)
    score = qhat + (multiplier * C) * sqrt(log(node->typedN) / (double)node->children[i]->typedN); // add exploration component

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
  type_vts *t = (type_vts *)void_t;
  treeNode *node = t->root;
  
  //travel down the tree using ucb
  while (_DOM->getGameStatus(node->rep) == INCOMPLETE && node->typedN > 0)
    node = node->children[selectMove_vts(node, C)];
 
  return node;
}

void assignToType_vts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
  type_system *ts = (type_system *)void_ts;
  int i;
  
  if (fatherType == -1) { //no father means root node
    ((type_vts *)ts->types[0])->root = node;
    ((type_vts *)ts->types[0])->birth = ts->visits;
    node->typeDefiner = true;
  } else {
    type_vts *t = (type_vts *)ts->types[fatherType];
    t->root->typeDefiner = false;
    //The size of a type is the size of the subtree rooted at the node representing the type
    //which is updated at the expansion phase, so only need to take care of splitting types
    if (t->root->typedSubtreeSize > threshold) {
      //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
      //is the ancestor of the current node to be inserted.
      //This ancestor will become the root of a new type which will be in the same index as the old type.
      treeNode *newFather = node;
      while (newFather->parent != t->root)
	newFather = newFather->parent;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	if (!_DOM->isValidChild(t->root->rep, t->root->side, i) || t->root->children[i] == newFather || t->root->children[i]->typeDefiner == true) //skip empty children or this is the new father (will be taken care of later) or if its already a type
	  continue;
	
	ts->numTypes++;
	ts->types = realloc(ts->types, ts->numTypes * sizeof(type_vts *)); //alocate a new type
	
	ts->types[ts->numTypes - 1] = calloc(1, sizeof(type_vts));
	((type_vts *)ts->types[ts->numTypes - 1])->root = t->root->children[i];
	((type_vts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
	((type_vts *)ts->types[ts->numTypes - 1])->scoreSum = t->root->children[i]->typedScoreSum;
	((type_vts *)ts->types[ts->numTypes - 1])->minmax = t->root->children[i]->minmax;
	
	if (policy == DELETE_VMAB)
	  ts->types[ts->numTypes - 1]->visits = 0;
	else
	  ts->types[ts->numTypes - 1]->visits = t->root->children[i]->typedN;
      }
      
      t->root = newFather;
      if (policy == DELETE_VMAB)
	t->visits = 0;
      else
	t->visits = newFather->typedN;
      
      t->birth = ts->visits;
      t->scoreSum = newFather->typedScoreSum;
      t->minmax = newFather->minmax;
    }
  } 
}

void destroy_vts(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++)
    free(ts->types[i]);
  
  free(ts->types);
}

void typeSignificance(type_system *ts, type_vts *type, treeNode **path) {
  int pathLength = path[0]->depth - type->root->depth;
  int i;
  double nodeMean; //mean of the current node;
  treeNode *node; //current node
  double z; //the current z-test value
  
  double shiftedTypeMean;
  
  for (i = pathLength - 1; i >= 0; i--) {
    node = path[i];
    
    //Split based on value only when there were more than X rollouts from this subtree.
    if (node->typedN < 30) //TODO param this?
      break;
    
    nodeMean = node->typedScoreSum / (double)node->typedN;
    
    //The mean of the root of the type, without the rollouts from the possibly new type.
    shiftedTypeMean = (type->root->typedScoreSum - node->typedScoreSum) / (double)(type->root->typedN - node->typedN);
    
    z = (nodeMean - shiftedTypeMean) / (sqrt(node->sd / node->typedN));
    
    if (z > 2.33) { //TODO: param this?
      ts->numTypes++;
      ts->types = realloc(ts->types, ts->numTypes * sizeof(type_vts *)); //alocate a new type
      
      //New Type
      ts->types[ts->numTypes - 1] = calloc(1, sizeof(type_vts));
      ((type_vts *)ts->types[ts->numTypes - 1])->root = node;
      node->typeDefiner = true;
      ((type_vts *)ts->types[ts->numTypes - 1])->birth = ts->visits;
      ((type_vts *)ts->types[ts->numTypes - 1])->scoreSum = node->typedScoreSum;
      ((type_vts *)ts->types[ts->numTypes - 1])->visits = node->typedN;
      
      //travers back the tree
      treeNode *bp = node->parent;
      double bpMean;
      double newTypeSumOfSquares = node->sd * (node->typedN - 1) + node->typedN * nodeMean * nodeMean;
      double nodeSumOfSquares;
      double updatedSumOfSquares;
      double updatedN;
      double updatedScoreSum;
      double updatedMean;
      double bestScore;
      while (bp != type->root->parent) {
	//Sum of squares of the large set.
	bpMean = bp->typedScoreSum / (double)(bp->typedN);
	nodeSumOfSquares = bp->sd * (bp->typedN - 1) + bp->typedN * bpMean * bpMean;
	
	updatedSumOfSquares = nodeSumOfSquares - newTypeSumOfSquares; //sum of squares of the new set
	
	updatedN = bp->typedN - node->typedN;
	updatedScoreSum = bp->typedScoreSum - node->typedScoreSum;
	updatedMean = updatedScoreSum / (double)updatedN;
	
	bp->sd = (updatedSumOfSquares - updatedN * updatedMean * updatedMean) / (double)(updatedN - 1);
	bp->typedN = updatedN;
	bp->typedScoreSum = updatedScoreSum;
	
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	    if (_DOM->isValidChild(bp->rep, bp->side, i) && bp->typeDefiner == false) { // if child exists, is it the best scoring child?
	      if ((bp->side == max && bp->children[i]->minmax > bestScore) || (bp->side == min && bp->children[i]->minmax < bestScore))
		bestScore = bp->minmax;
	    }
	}  
	bp->minmax = bestScore;
	
	bp = bp->parent;
      }
      
      //Old Type
      type->visits = type->root->typedN;
      type->scoreSum = type->root->typedScoreSum;
      break;
    }
  }
}