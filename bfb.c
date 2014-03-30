#include "common.h"
#include "type.h"
#include "type_reachability.h"
#define USE_MINIMAX_REWARDS 0

static int mm_Counter; // for counting nodes

//TODO extract and merge with uct.c
/* Routine to free up UCT tree */
static void freeTree(treeNode* node) {
  int i;
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (node->children[i]) {
      freeTree(node->children[i]);
      node->children[i] = NULL;
    }
  }
  _DOM->destructRep(node->rep);
  free(node->children);
  free(node);
}

static void freeTypeSystems(type_system **type_systems) {
  int i;
  for (i = 1; i < _DOM->getNumOfChildren(); i++)
    if (type_systems[i])
      destroy_ts(type_systems[i]);
    
  free(type_systems);
}

//Invoked by bfbIteration to select a type out of the type system based on UCB1
static int selectType(void *void_ts, double C, int side, int policy, int backupOp) {
  type_system *ts = (type_system *)void_ts;
  int i;
  double qhat;
  double score;
  int numBestTypes = 0;
  double bestScore = -INF;
  int bestTypes[ts->numTypes];
  int policyVisits;

  //if (USE_MINIMAX_REWARDS)
  //  C = averageValueOfAllTypes(ts);
  
  // The multiplier is used to set the sign of the exploration bonus term (should be negative
  // for the min player and positive for the max player) i.e. so that we correctly compute
  // an upper confidence bound for Max and a lower confidence bound for Min
  double multiplier = (side == max) ? 1 : -1;
  for (i = 0; i < ts->numTypes; i++) { // iterate over all types
    //If the type has never been visited before, select it first
    if (ts->types[i]->root->n == 0)
      return i;
    
    if (policy == MAB)
      policyVisits = ts->visits;
    else if (policy == KEEP_VMAB)
      policyVisits = ts->visits - ts->types[i]->birth + ts->types[i]->root->n;
    else if (policy == DELETE_VMAB)
      policyVisits = ts->visits - ts->types[i]->birth;
    
    // Otherwise, compute this type's UCB1 index (will be used to pick best type if it transpires that all
    // types have been visited at least once)
    if (USE_MINIMAX_REWARDS) {
      double avg = ts->types[i]->root->scoreSum / (double)ts->types[i]->root->n;

      //qhat = 1.0 - calcQhatBasedOnMinimax(ts,i) / (double)ts->types[i]->root->depth;
      qhat = 1.0 - (double)minmaxLevel(ts->types[i]->root) / (double)ts->types[i]->root->depth;
      
      //if (ts->visits == logiter)
	//printf("avg %f minmaxLevel(ts->types[i]->root) %f ts->types[i]->root->depth %d\n", avg, minmaxLevel(ts->types[i]->root), ts->types[i]->root->depth);
      C = 0;
      score = qhat + C * sqrt(log(policyVisits) / (double)ts->types[i]->root->n); // add exploration component (-1 for minimization)
      
    } else {
      qhat = ts->types[i]->root->scoreSum / (double)ts->types[i]->root->n;  // exploitation component (this is the average utility or minimax value)
      score = qhat + (multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->root->n); // add exploration component

      // Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
      // is min's turn means we can always just take the maximum
      score = (side == min) ? -score : score;
    }
    
    // If this is either the first child, or the best scoring child, store it
    if ((numBestTypes == 0) || (score > bestScore)) {
      bestTypes[0] = i;
      bestScore = score;
      numBestTypes = 1;
    } else if (score == bestScore) // if this child ties with the best scoring child, store it
      bestTypes[numBestTypes++] = i;
  }
  //if (ts->visits == logiter) {
   // printf("visits %d best %f\n",ts->visits,bestScore);
  //}
  // Return the next type to explore (break ties randomly)
  return bestTypes[random() % numBestTypes];
}

//Generate the i'th child of a uct node
static void generateChild(treeNode *node, int i) {
  mm_Counter++;
  node->children[i] = calloc(1, sizeof(treeNode));
  node->children[i]->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
  node->children[i]->rep = _DOM->cloneRep(node->rep); // copy over the current board to child
  node->children[i]->side = node->side; // copy over the current side on move to child
  _DOM->makeMove(node->children[i]->rep, &(node->children[i]->side), i); //Make the i-th move
  node->children[i]->parent = node; //save parent
  node->children[i]->depth = node->depth + 1;
  node->children[i]->subtreeSize = 1;
}

double actionCostFindMove(treeNode *node) {
  if (node->parent == NULL || isChanceNode_sailing(node->rep) == false)
    return 0;
  
  int i;
  for (i = 1; i < _DOM->getNumOfChildren(); i++)
    if (node->parent->children[i] == node)
      return actionCost_sailing(node->parent->rep, i);
    
  return 0;
}

//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(treeNode *root, double C, double CT, heuristics_t heuristic, int budget, int backupOp, int side, int threshold, int policy) { 
  treeNode *node = root;
  int generated = false;
  int move;
  double ret;
  
  //travel down the tree using ucb
  while ((ret = _DOM->getGameStatus(node->rep)) == INCOMPLETE && node->n > 0) {    
    //TODO handle chance node diffrently (domain independant)
    if (_DOM->dom_name == SAILING && isChanceNode_sailing(node->rep) == true) {
      move = selectMoveStochastic_sailing(node->rep);
    } else
      move = selectMove(node, C);

    //printf("depth %d scoresum %f n %d qhat %f\n",node->depth, node->scoreSum, node->n, node->scoreSum / (double)node->n);
    if (node->children[move] == NULL) {
      generateChild(node, move);
      generated = true;
    }
    
    node = node->children[move];
  }

  if (ret != INCOMPLETE) {
    if (_DOM->dom_name == SAILING)
	ret = 0;
    
    // This is a terminal node (i.e. can't generate any more children)
    // If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
    // those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
    // which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
    // node values. If we are using engineered heuristics, then no rescaling is necessary.
    if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5))
	ret /= MAX_WINS; // rescale
      
  } else {
    ret = heuristic(node->rep, node->side, budget); 
  }
 
  //backpropagate
  int aboveType = false;
  int i;
  while (node != NULL) {
    node->n++;
    if (generated)
      node->subtreeSize++;
    
    if (!aboveType) {
      node->scoreSum += ret;
    
      if (node->type == true) {
	//Split
	if (node->subtreeSize > threshold) {
	  node->type = false;
      
	  for (i = 1; i < _DOM->getNumOfChildren(); i++)
	    if (node->children[i])
	      node->children[i]->type = true;
	}
	
	//the node above the type is treated as a leaf in a minmax tree, thus we want to update the one above him.
	if (node->parent != NULL && node->subtreeSize <= threshold) {
	  //Add action cost for mdps
	  if (_DOM->dom_name == SAILING)
	    ret += actionCostFindMove(node);
    
	  node = node->parent;
	  node->n++;
	  node->scoreSum += ret;
	  if (generated)
	    node->subtreeSize++;
	}
	
	aboveType = true;
      }
      
    } else {
      double bestScore = (node->side == max) ? -INF : INF;
      double score;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	if (node->children[i] && node->children[i]->type == false) { // if child exists, is it the best scoring child?
	  score = node->children[i]->scoreSum / (double)node->children[i]->n;
	  if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore)))
	    bestScore = score;
	}
      }

      node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
    }
    
    //Add action cost for mdps
    if (_DOM->dom_name == SAILING)
      ret += actionCostFindMove(node);

    node = node->parent;
  }
}

int makeBFBMove(rep_t rep, int *side, int tsId, int numIterations, double C, double CT, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp, int threshold, int policy) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;
  
  *numBestMoves = 0; // reset size of set of best moves
  
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  rootNode->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
  rootNode->rep = _DOM->cloneRep(rep);
  rootNode->side = *side;
  rootNode->depth = 0;
  rootNode->subtreeSize = 1;
  rootNode->type = true;

  //Run specified number of BFB iterations
  for (i = 0; i < numIterations; i++) {
    //printf("ITERATION %d\n", i);
    bfbIteration(rootNode, C, CT, heuristic, budget, backupOp, *side, threshold, policy);
  }

  //Now look at the children 1-ply deep and determing the best one (break ties randomly)
  for (i = 1; i < _DOM->getNumOfChildren(); i++) { //For each move
    if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) //Skip illegal moves
      continue;

    if (!rootNode->children[i]) //This node was not created since # iterations was too small
      continue;

    //Compute average utility of this child
    val = rootNode->children[i]->scoreSum / (double)rootNode->children[i]->n;
    if (_DOM->dom_name == SAILING)
      val += actionCost_sailing(rootNode->rep, i);
    //If this was min's move, negate the utility value (this makes things a little cleaner
    // as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
    val = (*side == min) ? -val : val;

    //If this is the first child, or the best scoring child, then store it
    if ((*numBestMoves == 0) || (val > bestScore)) {
      bestMoves[0] = i;
      bestScore = val;
      *numBestMoves = 1;
    }
    else if (val == bestScore) //Child ties with currently best scoring one; store it
      bestMoves[(*numBestMoves)++] = i;

    if (verbose)
      printf("Move # %d -- Value %f, Count %d\n", i, ((*side == min) ? -val : val), rootNode->children[i]->n);
  }

  //We should have at least looked at one child
  assert(*numBestMoves != 0);

  //bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
  bestMove = bestMoves[0];

  if (verbose) {
    printf("Value of root node: %f\n", rootNode->scoreSum / (double)rootNode->n);
    printf("Best move: %d\n", bestMove);
  }

  // Clean up before returning
  freeTree(rootNode);
  
  return bestMove;
}

void printBfbStats(){
	printf("** Total BFB nodes %d**\n",mm_Counter);
}