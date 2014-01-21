#include "common.h"
#include "type.h"

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
      type_systems[i]->destroy(type_systems[i]);
    
  free(type_systems);
}


//TODO merge with select move of uct.c and sts.c
//Select a child of a uct node based on ucb1
static int selectMove(treeNode* node, double C) {
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

//Invoked by bfbIteration to select a type out of the type system based on UCB1
static int selectType(void *void_ts, double C, int side, int policy) {
  type_system *ts = (type_system *)void_ts;
  int i;
  double qhat;
  double score;
  int numBestTypes = 0;
  double bestScore;
  int bestTypes[ts->numTypes];
  
  // The multiplier is used to set the sign of the exploration bonus term (should be negative
  // for the min player and positive for the max player) i.e. so that we correctly compute
  // an upper confidence bound for Max and a lower confidence bound for Min
  double multiplier = (side == max) ? 1 : -1;
  int policyVisits;
  for (i = 0; i < ts->numTypes; i++) { // iterate over all types
    //If the type has never been visited before, select it first
    if (ts->types[i]->visits == 0)
      return i;

    if (policy == MAB)
      policyVisits = ts->visits;
    else if (policy == KEEP_VMAB)
      policyVisits = ts->visits - ts->types[i]->birth + ts->types[i]->visits;
    else if (policy == DELETE_VMAB)
      policyVisits = ts->visits - ts->types[i]->birth;
    
    // Otherwise, compute this type's UCB1 index (will be used to pick best type if it transpires that all
    // types have been visited at least once)
    qhat = ts->types[i]->scoreSum / (double)ts->types[i]->visits;  // exploitation component (this is the average utility or minimax value)
    score = qhat + (multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->visits); // add exploration component
    
    // Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
    // is min's turn means we can always just take the maximum
    score = (side == min) ? -score : score;
    
    // If this is either the first child, or the best scoring child, store it
    if ((numBestTypes == 0) || (score > bestScore)) {
      bestTypes[0] = i;
      bestScore = score;
      numBestTypes = 1;
    }
    else if (score == bestScore) // if this child ties with the best scoring child, store it
      bestTypes[numBestTypes++] = i;
  }
  
  // Return the next type to explore (break ties randomly)
  return bestTypes[random() % numBestTypes];
}

//Generate the i'th child of a uct node
static void generateChild(treeNode *node, int i) {
  node->children[i] = calloc(1, sizeof(treeNode));
  node->children[i]->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
  node->children[i]->rep = _DOM->cloneRep(node->rep); // copy over the current board to child
  node->children[i]->side = node->side; // copy over the current side on move to child
  _DOM->makeMove(node->children[i]->rep, &(node->children[i]->side), i); //Make the i-th move
  node->children[i]->parent = node; //save parent
  node->children[i]->depth = node->depth + 1;
  node->children[i]->subtreeSize = 1;
}

//TODO: change the way you handle empty open lists - should stop the main iteration loop and also maybe should not be returned from here or from select type
//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(type_system *ts, double C, heuristics_t heuristic, int budget, int backupOp, int side, int threshold, int policy) { 
  int i;
  
  //Choose type and node from the chosen type
  int typeId = selectType(ts, C, side, policy);
  
  if (typeId == -1)  //Open lists are all empty
    return;
  
  type *t = ts->types[typeId];
  
  treeNode *node = ts->selectFromType(t, C);

  double ret;
  int gameOver = 0;
  if ((ret = _DOM->getGameStatus(node->rep)) != INCOMPLETE) {
    gameOver = 1;
    // This is a terminal node (i.e. can't generate any more children)
    // If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
    // those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
    // which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
    // node values. If we are using engineered heuristics, then no rescaling is necessary.
    if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5))
      ret /= MAX_WINS; // rescale
  } else
    ret = heuristic(node->rep, node->side, budget);
  
  //update type ucb stats
  t->visits++;
  if (backupOp == AVERAGE) {
    t->scoreSum = t->scoreSum + ret;
    
    if (ts->name == VTS)
      ((type_vts *)t)->deviationSum = ((type_vts *)t)->deviationSum + (ret - t->scoreSum) * (ret - t->scoreSum); //TODO fix formula
  } else if (backupOp == MINMAX) {
    if (t->visits == 1)
      t->scoreSum = ret;
    else {
      double score = t->scoreSum / (double)(t->visits - 1);
      if ((side == max && ret > score) || (side == min && ret < score))
	t->scoreSum = ret * t->visits;
    }
  }
  
  //backpropagate
  treeNode *bp = node;
  while (bp != NULL) {
    bp->n++;
    
    if (backupOp == AVERAGE) {
      bp->scoreSum = bp->scoreSum + ret;
      bp->deviationSum = bp->deviationSum + (bp->scoreSum - ret) * (bp->scoreSum - ret); //TODO fix formula
      
    } else if (backupOp == MINMAX) {
      //If first visit to node than the value is set by default      
      if (bp->n == 1) 
	bp->scoreSum = ret;
      else if (gameOver == 1 && bp == node)
	bp->scoreSum = bp->scoreSum + ret;
      else { //compute minimax
	double bestScore = (bp->side == max) ? MIN_WINS : MAX_WINS;
	
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	  if (_DOM->isValidChild(bp->rep, bp->side, i)) { // if child exists, is it the best scoring child?
	    double score = bp->children[i]->scoreSum / (double)(bp->children[i]->n);
	    if ((bp->side == max && score > bestScore) || (bp->side == min && score < bestScore))
	      bestScore = score;
	  }
	}
	
	bp->scoreSum = bestScore * bp->n;
      }
    }
    
    bp = bp->parent;
    
  }
   
  if (gameOver) //terminal node - no need to expand
    return;
  
  //generate the children and place them in the type system
  int numOfChildren = 0; //number of children actually generated
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (!_DOM->isValidChild(node->rep, node->side, i)) // if the i^th move is illegal, skip it
      continue;
    
    generateChild(node, i);
    ts->assignToType(ts, node->children[i], typeId, threshold, policy);
    numOfChildren++;
  }
  
  //Update the size of the sub tree of each ancestor of the chosen node
  treeNode **path; //path from type root node to the chosen frontier node
  if (ts->name == VTS) {
    path = calloc(node->depth - ((type_vts *)t)->root->depth, sizeof(treeNode *));
    ((type_vts *)t)->size = ((type_vts *)t)->size + numOfChildren; //in vts the size of the type isnt in the root node as in sts.
  }
  
  i = 0;
  bp = node;
  while (bp != NULL) {
    bp->subtreeSize = bp->subtreeSize + numOfChildren;
    bp = bp->parent;
    
    if (ts->name == VTS) {
      if (bp == ((type_vts *)t)->root) //no need to log path anymore
	i = -1;
      else if (i != -1)
	path[i++] = bp; 
    }
  }
  
  if (ts->name == VTS) {
    typeSignificance(ts, (type_vts *)t, path);
    free(path);
  }
}

int makeBFBMove(rep_t rep, int *side, int tsId, int numIterations, double C, heuristics_t heuristic, int budget,
		int* bestMoves, int* numBestMoves, int backupOp, int threshold, int policy) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;
  type_system **type_systems = calloc(_DOM->getNumOfChildren(), sizeof(type_system *)); //Alocate a type system for every possible child of the root
  
  *numBestMoves = 0; // reset size of set of best moves
  
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  rootNode->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
  rootNode->rep = _DOM->cloneRep(rep);
  rootNode->side = *side;
  rootNode->depth = 0;
  rootNode->subtreeSize = 1;
  
  //Generate the children of the root node
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) // if the i^th move is illegal, skip it
      continue;
    
    generateChild(rootNode, i);
    rootNode->subtreeSize++;
    
    //Allocate aa type system for the i'th child and assign him to it
    type_systems[i] = init_type_system(tsId, rep, *side);
    type_systems[i]->assignToType(type_systems[i], rootNode->children[i], -1, threshold, policy);
  }

  //Run specified number of BFB iterations
  //Starts at one because i is also used as the total number of visits
  for (i = 1; i < numIterations + 1; i++) {
    type_system *ts = type_systems[selectMove(rootNode, C)];
    ts->visits++;
    bfbIteration(ts, C, heuristic, budget, backupOp, *side, threshold, policy);
  }

  //Now look at the children 1-ply deep and determing the best one (break ties randomly)
  for (i = 1; i < _DOM->getNumOfChildren(); i++) { //For each move
    if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) //Skip illegal moves
      continue;

    if (!rootNode->children[i]) //This node was not created since # iterations was too small
      continue;

    //Compute average utility of this child
    val = rootNode->children[i]->scoreSum / (double)rootNode->children[i]->n;

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

  bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
  //bestMove = bestMoves[0];
  _DOM->makeMove(rep, side, bestMove); // make it (updates game state)

  if (verbose) {
    printf("Value of root node: %f\n", rootNode->scoreSum / (double)rootNode->n);
    printf("Best move: %d\n", bestMove);
  }

  // Clean up before returning
  freeTree(rootNode);
  freeTypeSystems(type_systems);
  
  return bestMove;
}