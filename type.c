#include "type.h"
#define SIMPLE_REGRET_UCT 0

//TODO merge with select move of uct.c  Note the duplication of 3(!) identical methods. 

/**
 * Exploitation term acording to the uct formula 
 */ 
static double uctExploration(double multiplier, double C, treeNode* node, int i){
	return (multiplier * C) * sqrt(log(node->n) / (double)node->children[i]->n); // add exploration component
}
/**
 * Exploitation term acording to the simple regret uct formula 
 */ 
static double uctExplorationSimpleRegret(double multiplier, double C, treeNode* node, int i){
	return 	sqrt((multiplier * C) * sqrt(node->n)/(double)node->children[i]->n); // add exploration component
}

void destroy_ts(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++)    
    free(ts->types[i]);
  
  free(ts->types);
}

//Select a child of a uct node based on ucb1
int selectMove(treeNode* node, double C) {
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
    if (node->children[i] == NULL || node->children[i]->n == 0)
      return i;
    // Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
    // children have been visited at least once)
    qhat = node->children[i]->scoreSum / (double)node->children[i]->n;  // exploitation component (this is the average utility)
    if (SIMPLE_REGRET_UCT){
	    score = qhat + uctExplorationSimpleRegret(multiplier,C,node,i);
    } else {
	    score = qhat + uctExploration(multiplier,C,node,i);
    }
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
  //printf("scoresum %f n %d qhat %f\n",node->children[i]->scoreSum,node->children[i]->n, qhat);
  //printf("depth %d best socre %f\n",node->depth, bestScore);
  return bestMoves[0];
}


void *init_type_system(int t) {
  type_system *ts = calloc(1, sizeof(type_system));
  
  switch (t) {   
    case STS:
      ts->name = STS;
      ts->assignToType = assignToType_sts;
      ts->numTypes = 1;
      ts->types = calloc(ts->numTypes, sizeof(type *));
      ts->types[0] = calloc(1, sizeof(type));
      break;
  }
  return ts;
}