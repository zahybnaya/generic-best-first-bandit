#include "common.h"
#include "type.h"

static int mm_Counter; // for counting nodes

//TODO extract and merge with uct.c
/* Routine to free up UCT tree */
static void freeTree(treeNode* node) {
  int i;
  for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
    if (node->children[i]) {
      freeTree(node->children[i]);
      node->children[i] = NULL;
    }
  }
  _DOM->destructRep(node->rep);
  free(node->children);
  free(node);
}

static void freeTypeSystems(type_system **type_systems, int size) {
  int i;
  for (i = 1; i < size; i++)
    if (type_systems[i])
      destroy_ts(type_systems[i]);
    
  free(type_systems);
}

//return a random number in [0, 1]
static double unitRand() {
  return (random() % 10000000) / (double)10000000;
}

//Invoked by bfbIteration to select a type out of the type system based on UCB1
static int selectType(type_system *ts, double C, int side, int policy, int backupOp) {
  int i;
  double qhat;
  double score;
  int numBestTypes = 0;
  double bestScore = -INF;
  int bestTypes[ts->numTypes];
  int policyVisits;
  double scaledAvgSum = 0;
  double scaledAvgs[ts->numTypes];
  
  // The multiplier is used to set the sign of the exploration bonus term (should be negative
  // for the min player and positive for the max player) i.e. so that we correctly compute
  // an upper confidence bound for Max and a lower confidence bound for Min
  double multiplier = (side == max) ? 1 : -1;
  for (i = 0; i < ts->numTypes; i++) { // iterate over all types
    //If the type has never been visited before, select it first
    if (ts->types[i]->n == 0)
      return i;
    
    if (policy == MAB)
      policyVisits = ts->visits;
    else if (policy == KEEP_VMAB)
      policyVisits = ts->visits - ts->birthdays[i] + ts->types[i]->n;
    else if (policy == DELETE_VMAB)
      policyVisits = ts->visits - ts->birthdays[i];
    
    // Otherwise, compute this type's UCB1 index (will be used to pick best type if it transpires that all
    // types have been visited at least once)
    qhat = ts->types[i]->scoreSum / (double)ts->types[i]->n;  // exploitation component (this is the average utility or minimax value)
    
    if (CHOOSE_TYPE == UCB) {
      score = qhat + (multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->n); // add exploration component
    } else if (CHOOSE_TYPE == E_GREEDY) {
      score = qhat;
    } else if (CHOOSE_TYPE == PROB_AND_AVG || CHOOSE_TYPE == PROB_AND_AVG_AND_EXPLORE || CHOOSE_TYPE == E_GREEDY_PATH_PROB) {
      scaledAvgs[i] = qhat * ts->types[i]->pathProb;
      scaledAvgSum += scaledAvgs[i];
      continue;
    }

    // Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
    // is min's turn means we can always just take the maximum
    score = (side == min) ? -score : score;
    
    // If this is either the first child, or the best scoring child, store it
    if ((numBestTypes == 0) || (score > bestScore)) {
      bestTypes[0] = i;
      bestScore = score;
      numBestTypes = 1;
    } else if (score == bestScore) // if this child ties with the best scoring child, store it
      bestTypes[numBestTypes++] = i;
  }

  if (CHOOSE_TYPE == E_GREEDY) {
    if (!(unitRand() < 1 - GREEDY_EPSILON))
      return random() % ts->numTypes;
  } else if (CHOOSE_TYPE == E_GREEDY_PATH_PROB) {
    if (scaledAvgSum == 0)
      return 0;
    
    double bestPathProb = -INF;
    for (i = 0; i < ts->numTypes; i++) { 
      double currentPathProb = scaledAvgs[i] / scaledAvgSum;
      if (currentPathProb > bestPathProb) {
	bestPathProb = currentPathProb;
	bestTypes[0] = i;
      }
    }
    
    if (!(unitRand() < 1 - GREEDY_EPSILON))
      return random() % ts->numTypes;
    
  } else if (CHOOSE_TYPE == PROB_AND_AVG) {
    if (scaledAvgSum == 0)
      return 0;
    
    double rand = unitRand();
    double accumulateProb = 0;
    for (i = 0; i < ts->numTypes; i++) {
      accumulateProb += (scaledAvgs[i] / scaledAvgSum);
      //printf("ts->types[i]->pathProb %f ts->numTypes %d scaledAvgs[i] %f scaledAvgSum %f accumulateProb %f rand %f\n", ts->types[i]->pathProb, ts->numTypes, scaledAvgs[i], scaledAvgSum, accumulateProb, rand);
      if (rand <= accumulateProb)
	return i;
    }
  } else if (CHOOSE_TYPE == PROB_AND_AVG_AND_EXPLORE) { 
    if (scaledAvgSum == 0)
      return 0;
    
    for (i = 0; i < ts->numTypes; i++) { 
      if (policy == MAB)
	policyVisits = ts->visits;
      else if (policy == KEEP_VMAB)
	policyVisits = ts->visits - ts->birthdays[i] + ts->types[i]->n;
      else if (policy == DELETE_VMAB)
	policyVisits = ts->visits - ts->birthdays[i];
    
      C = 1 / sqrt(2);
      qhat = scaledAvgs[i] / scaledAvgSum;
      score = qhat + (multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->n);

      score = (side == min) ? -score : score;
    
      // If this is either the first child, or the best scoring child, store it
      if ((numBestTypes == 0) || (score > bestScore)) {
	bestTypes[0] = i;
	bestScore = score;
	numBestTypes = 1;
      } else if (score == bestScore) // if this child ties with the best scoring child, store it
	bestTypes[numBestTypes++] = i;
    }
  }
  
  return bestTypes[0];
}

//Generate the i'th child of a uct node
static void generateChild(treeNode *node, int i) {
  mm_Counter++;
  node->children[i] = calloc(1, sizeof(treeNode));
  node->children[i]->rep = _DOM->cloneRep(node->rep); // copy over the current board to child
  node->children[i]->side = node->side; // copy over the current side on move to child
  _DOM->makeMove(node->children[i]->rep, &(node->children[i]->side), i); //Make the i-th move
  node->children[i]->children = calloc(_DOM->getNumOfChildren(node->rep, node->side), sizeof(treeNode*));
  node->children[i]->parent = node; //save parent
  node->children[i]->depth = node->depth + 1;
  node->children[i]->subtreeSize = 1;
  node->children[i]->pathProb = node->pathProb;
  
  if (_DOM->dom_name == SAILING && isChanceNode_sailing(node->rep) == true) {
    if (((int *)(node->rep))[WIND] == ((int *)(node->children[i]->rep))[WIND]) //No wind change
      node->children[i]->pathProb *= 1 - 2 * SAILING_WIND_CHANGE_PROB;
    else 
      node->children[i]->pathProb *= SAILING_WIND_CHANGE_PROB;
  }  
}

double actionCostFindMove(treeNode *node) {
  if (node->parent == NULL || isChanceNode_sailing(node->rep) == false)
    return 0;
  
  int i;
  for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++)
    if (node->parent->children[i] == node)
      return actionCost_sailing(node->parent->rep, i);
    
  return 0;
}

//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(type_system *ts, treeNode *root, double C, double CT, heuristics_t heuristic, int budget, int backupOp, int side, int threshold, int policy) {   
  
  treeNode *node;
  int typeId = -1;
  if (TYPE_FROM_ROOT) {
     node = root;
  } else {
    typeId = selectType(ts, C, side, policy, backupOp);
    node = ts->types[typeId];
  }
  
  int move;
  int generated = false;
  //travel down the tree using ucb
  while (_DOM->getGameStatus(node->rep) == INCOMPLETE && node->n > 0) {    
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
  
  double ret;
  if ((ret = _DOM->getGameStatus(node->rep)) != INCOMPLETE) {
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
  ts->backprop(ts, node, ret, generated, typeId, threshold);
}

int makeBFBMove(rep_t rep, int *side, int tsId, int numIterations, double C, double CT, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp, int threshold, int policy) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;
  type_system **type_systems = calloc(_DOM->getNumOfChildren(rep, *side), sizeof(type_system *)); //Alocate a type system for every possible child of the root
  *numBestMoves = 0; // reset size of set of best moves
  
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  rootNode->children = calloc(_DOM->getNumOfChildren(rep, *side), sizeof(treeNode*));
  rootNode->rep = _DOM->cloneRep(rep);
  rootNode->side = *side;
  rootNode->depth = 0;
  rootNode->subtreeSize = 1;
  rootNode->type = false;
  rootNode->pathProb = 1;
  rootNode->scoreSum = heuristic(rootNode->rep, rootNode->side, budget);
  rootNode->n++;
  
  if (!TYPE_FROM_ROOT) {
    //Generate the children of the root node
    for (i = 1; i < _DOM->getNumOfChildren(rep, *side); i++) {
      if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) // if the i^th move is illegal, skip it
	continue;
    
      generateChild(rootNode, i);
      rootNode->subtreeSize++;
    
      //Allocate a type system for the i'th child and assign him to it
      type_systems[i] = init_type_system(tsId);
      type_systems[i]->types[0] = rootNode->children[i];
      rootNode->children[i]->type = true;
    }
  }
  
  //Run specified number of BFB iterations (gets one iteration less because of the root is expanded before hand)
  for (i = 0; i < numIterations - 1; i++) {
    //printf("ITERATION %d\n", i);
    type_system *ts = 0;
    if (!TYPE_FROM_ROOT) {  
      int move = selectMove(rootNode, C);
      ts = type_systems[move];
      ts->visits++;
    }
    bfbIteration(ts, rootNode, C, CT, heuristic, budget, backupOp, *side, threshold, policy);
  }

  //Now look at the children 1-ply deep and determing the best one (break ties randomly)
  for (i = 1; i < _DOM->getNumOfChildren(rootNode->rep, rootNode->side); i++) { //For each move
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
  freeTypeSystems(type_systems, _DOM->getNumOfChildren(rootNode->rep, rootNode->side));
  
  return bestMove;
}

void printBfbStats(){
	printf("** Total BFB nodes %d**\n",mm_Counter);
}