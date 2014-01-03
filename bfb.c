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

//TODO: change the way you handle empty open lists - should stop the main iteration loop and also maybe should not be returned from here or from select type
//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(type_system *ts, int visits, double C, heuristics_t heuristic, int budget, int backupOp, int side, int threshold) { 
  int i;
  
  //Choose type and node from the chosen type
  int typeId = ts->selectType(ts, C, visits, side);
  
  if (typeId == -1)  //Open lists are all empty
    return;
  
  type *t = ts->types[typeId];
  
  treeNode *node = ts->selectFromType(t);

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
  if (backupOp == AVERAGE)
    t->scoreSum = t->scoreSum + ret;
  else if (backupOp == MINMAX) {
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
    
    if (backupOp == AVERAGE)
      bp->scoreSum = bp->scoreSum + ret;
    else if (backupOp == MINMAX) {
      //If first visit to node than the value is set by default      
      if (bp->n == 1) 
	bp->scoreSum = ret;
      else if (gameOver == 1 && bp == node)
	bp->scoreSum = bp->scoreSum + ret;
      else { //Else, compute minimax
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
    
    node->children[i] = calloc(1, sizeof(treeNode));
    node->children[i]->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
    node->children[i]->rep = _DOM->cloneRep(node->rep); // copy over the current board to child
    node->children[i]->side = node->side; // copy over the current side on move to child
    _DOM->makeMove(node->children[i]->rep, &(node->children[i]->side), i); //Make the i-th move
    node->children[i]->parent = node; //save parent
    node->children[i]->depth = node->depth + 1;
    ts->assignToType(ts, node->children[i], typeId, threshold);
    node->children[i]->subtreeSize = 1;
    numOfChildren++;
  }
  
  //Update the size of the sub tree of each ancestor of the chosen node
  bp = node;
  while (bp != NULL) {
    bp->subtreeSize = bp->subtreeSize + numOfChildren;
    bp = bp->parent;
  }
}

int makeBFBMove(rep_t rep, int *side, void *void_ts, int numIterations, double C, heuristics_t heuristic, int budget,
		int* bestMoves, int* numBestMoves, int backupOp, int threshold) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;
  type_system *ts = (type_system *)void_ts;
  
  *numBestMoves = 0; // reset size of set of best moves
  
  ts->furtherInit(ts, rep, *side); //finish intilaizing the type system
  
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  rootNode->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
  rootNode->rep = _DOM->cloneRep(rep);
  rootNode->side = *side;
  rootNode->depth = 0;
  rootNode->subtreeSize = 1;
  
  //Assign the root to a type
  ts->assignToType(ts, rootNode, -1, threshold);

  //Run specified number of BFB iterations
  //Starts at one because i is also used as the total number of visits
  for (i = 1; i < numIterations + 1; i++)
    bfbIteration(ts, i, C, heuristic, budget, backupOp, *side, threshold);

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
  destroyTypeSystem(ts);
  
  return bestMove;
}