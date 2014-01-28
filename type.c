#include "type.h"
#define SIMPLE_REGRET_UCT 1

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
    if (node->children[i]->n == 0)
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
  return bestMoves[0];
}


void *init_type_system(int t, rep_t rep, int side) {
  type_system *ts = calloc(1, sizeof(type_system));
  
  switch (t) {
    case MM_ORACLE:     
      ts->name = MM_ORACLE;
      ts->assignToType = assignToType_mmOracle;
      ts->selectFromType = selectFromType_mmOracle;
      ts->destroy = destroy_mmOracle;
      
      ts->numTypes = MAX_WINS * 2 + 1; //Number of possible minmax values
      ts->types = calloc(ts->numTypes, sizeof(type_mmOracle *));
      
      int i;
      for (i = 0; i < ts->numTypes; i++) {
	ts->types[i] = calloc(1, sizeof(type_mmOracle));
	((type_mmOracle *)ts->types[i])->empty = -1;
      }
      
      ts->extra = calloc(1, sizeof(int));
      *(int *)(ts->extra) = open(ORACLE_PATH, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
      storeMinimax(*(int *)(ts->extra), rep, 0, ORACLE_DEPTH, side, ORACLE_H, 0);
      break;
      
    case STS:
      ts->name = STS;
      ts->assignToType = assignToType_sts;
      ts->selectFromType = selectFromType_sts;
      ts->destroy = destroy_sts;
      ts->numTypes = 1;
      ts->types = calloc(ts->numTypes, sizeof(type_sts *));
      ts->types[0] = calloc(1, sizeof(type_sts));
      break;
      
    case VTS:
      ts->name = VTS;
      ts->assignToType = assignToType_vts;
      ts->selectFromType = selectFromType_vts;
      ts->destroy = destroy_vts;
      
      ts->numTypes = 1;
      ts->types = calloc(ts->numTypes, sizeof(type_vts *));
      ts->types[0] = calloc(1, sizeof(type_vts));
      break;
  }
  return ts;
}
