#include "common.h"
#include "domain.h"
extern DOM* _DOM;


/* Routines related to the Minimax algorithm, with alpha-beta pruning */
// File scope globals
static int numNodes; // tracks number of nodes visited
static short dotFormat = false; // determines whether the search tree is printed out in dot format
static int id = 0; // used to set node ids (for graph visualization)
static int termCount = 0; // used to count number of terminal nodes encountered during a minimax search

/* Recursively compute the minimax value of a given node, with alpha-beta pruning. The search is cutoff
   at a maximum depth given by depth. searchDepth keeps track of how deep we already are in the tree */ 
static double alphaBeta(rep_t rep, int searchDepth, int depth, int side, double alpha, 
			double beta,heuristics_t heuristic,
		       	int budget,
			int currentNodeId) {
  double val1, val2;
  int i;
  int origSide = side;
  rep_t dummyRep;
  numNodes++; // track number of nodes expanded
  
  // Hit terminal node -- return value is one of {MAX_WINS, MIN_WINS, DRAW}
  if ((val1 = _DOM->getGameStatus(rep)) != INCOMPLETE) {
    termCount++; // update terminal node count
    if (dotFormat)
      printf("n%d [color=\"red\"];", currentNodeId); // terminal nodes are colored red
    return val1;
  }

  // Hit search depth cutoff -- apply the chosen heuristic function to this node and return that value
  if (searchDepth >= depth) {

    val1 = heuristic(rep, side, budget);

    // Ensure that the heuristic value is always bounded by the values assigned to terminal nodes (i.e. true win/loss
    // positions)
    assert ((val1 < MAX_WINS) && (val1 > MIN_WINS));

    return val1;
  }

  // Otherwise, recurse
  if (side == max) { // at a Max node
    val1 = alpha;
    for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i))
	      continue;
      dummyRep = _DOM->cloneRep(rep);// clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i);// generate i^th child
      // About to visit a new node, so add the edge to the graph
      if (dotFormat)
	printf("n%d -> n%d;\n", currentNodeId, ++id);
      val2 = alphaBeta(dummyRep , searchDepth+1, depth, side, val1, beta, heuristic, budget, id); // compute minimax value of this child
      val1 = (val2 > val1) ? val2 : val1; // we have tightened our alpha bound
      if (val1 >= beta) // we have a beta-cutoff 
	return beta;
    }
  }
  else { // at a Min node
    val1 = beta;
    for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i))
	continue;
      dummyRep = _DOM->cloneRep(rep);// clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i);// generate i^th child
      // About to visit a new node, so add the edge to the graph
      if (dotFormat)
	printf("n%d -> n%d;\n", currentNodeId, ++id);
      val2 = alphaBeta(dummyRep, searchDepth+1, depth, side, alpha, val1, heuristic, budget, id); // compute minimax value of this child
      val1 = (val2 < val1) ? val2 : val1; // we have tightened our beta bound
      if (val1 <= alpha) // we have an alpha-cutoff
	return alpha;
    }
  }

  return val1;
}

/* Recursively compute the minimax value of a given node, without alpha-beta pruning. The search is cutoff
   at a maximum depth given by depth. searchDepth keeps track of how deep we already are in the tree */ 
static double noPruningMM(rep_t rep, int searchDepth, int depth, int side, heuristics_t heuristic, int budget, int currentNodeId) {
  double val1, val2;
  int i;
  rep_t dummyRep;
  int origSide = side;

  numNodes++; // track number of nodes expanded
  
  // Hit terminal node -- return value is one of {MAX_WINS, MIN_WINS, DRAW}
  if ((val1 = _DOM->getGameStatus(rep)) != INCOMPLETE) {
    termCount++; // update terminal node count
    if (dotFormat)
      printf("n%d [color=\"red\"];", currentNodeId); // terminal nodes are colored red
    return val1;
  }

  // Hit search depth cutoff -- apply the chosen heuristic function to this node and return that value
  if (searchDepth >= depth) {

    val1 = heuristic(rep, side, budget);
    // Ensure that the heuristic value is always bounded by the values assigned to terminal nodes (i.e. true win/loss
    // positions)
    assert ((val1 < MAX_WINS) && (val1 > MIN_WINS));

    return val1;
  }

  // Otherwise, recurse
  if (side == max) { // at a Max node
    val1 = -INF;
    for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i))
	continue;
      dummyRep = _DOM->cloneRep(rep);// clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i);// generate i^th child

      // About to visit a new node, so add the edge to the graph
      if (dotFormat)
	printf("n%d -> n%d;\n", currentNodeId, ++id);

      val2 = noPruningMM(dummyRep, searchDepth+1, depth, side, heuristic, budget, id); // compute minimax value of this child
      val1 = (val2 > val1) ? val2 : val1; // we have tightened our alpha bound
    }
  }
  else { // at a Min node
    val1 = INF;
    for (i = 1; i < NUM_PITS+1; i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i))
	continue;
      dummyRep = _DOM->cloneRep(rep);// clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i);// generate i^th child

      // About to visit a new node, so add the edge to the graph
      if (dotFormat)
	printf("n%d -> n%d;\n", currentNodeId, ++id);

      val2 = noPruningMM(dummyRep , searchDepth+1, depth, side, heuristic, budget, id); // compute minimax value of this child
      val1 = (val2 < val1) ? val2 : val1; // we have tightened our beta bound
    }
  }

  return val1;
}

/* Perform alpha-beta search from given board position, and make the best move
   The set of bestMoves is returned via parameter bestMoves[] */
int makeMinmaxMove(rep_t rep, int* side, int depth, heuristics_t heuristic, int budget, int pruning,
	       	int randomTieBreaks, int noisyMM, int* bestMoves, int* numBestMoves, double* termPercentage) {
  int i;
  rep_t dummyRep;
  double val;
  int bestMove = NULL_MOVE;
  int secondBestMove = NULL_MOVE;
  double bestScore;
  double secondBestScore = -INF;
  int origSide = *side;
  double scores[_DOM->getNumOfChildren()];
  double die;

  numNodes = 0; // reset node count
  *numBestMoves = 0; // reset number of best moves
  termCount = 0; // reset terminal nodes count
  
  for (i = 1; i <_DOM->getNumOfChildren(); i++) { // iterate over all possible moves
    *side = origSide;
    if (!_DOM->isValidChild(rep,*side,i))
	continue;
   dummyRep = _DOM->cloneRep(rep);// clone starting board so it can be restored on next pass through loop
   _DOM->makeMove(dummyRep,side,i);// generate i^th child

    // Compute MM-(k-1) value of i^th child (since the children are already at depth 1 from the root node,
    // and we do a search from each child)
    if(pruning) //Alon
      val = alphaBeta(dummyRep , 1, depth, *side, MIN_WINS, MAX_WINS, heuristic, budget, id);
    else
      val = noPruningMM(dummyRep , 1, depth, *side, heuristic, budget, id);

    // If this was min's move, negate the utility value (this makes things a little cleaner
    // as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))    
    scores[i] = (origSide == min) ? -val : val;

    // If this is either the first child, or the best scoring child, store it
    if ((*numBestMoves == 0) || (scores[i] > bestScore)) {
      bestMoves[0] = i;
      bestScore = scores[i];
      *numBestMoves = 1;
    }
    // If this child ties with the best scoring child, and random tie-breaking is enabled, store it
    else if ((scores[i] == bestScore) && (randomTieBreaks))
      bestMoves[(*numBestMoves)++] = i;

    if (verbose)
      printf("Move # %d -- Value %f\n", i, val);
  }
  *side = origSide;
  bestMove = bestMoves[random() % *numBestMoves]; // pick a best move at random among the set of best moves

  if (verbose)
    printf("Value of root node: %f\n", (origSide == min) ? -bestScore : bestScore);

  // Find second best move -- note that the score of the second best move must be within 3 of the best move
  // In case there is no such second best move, the index remains set to its initial value of NULL_MOVE
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (!_DOM->isValidChild(rep,*side,i))
	continue;
    if (scores[i] != bestScore) {
      if ((bestScore - scores[i] <= 3.0) && (scores[i] > secondBestScore)) {
	secondBestScore = scores[i];
	secondBestMove = i;
      }
    }
  }

  if (verbose)
    printf("Best move: %d \t Second best move: %d\n", bestMove, secondBestMove);

  // If 'noisy MM' option was selected at command-line and there is a candidate second-best move,
  // flip a coin and determine whether to pick that over the best move
  if ((secondBestMove != NULL_MOVE) && (noisyMM)) {
    assert(heuristic != (heuristics_t)_DOM->hFunctions.h3); // could mess up if using playouts to estimate leaves, so flag for now
    die = (double)(random() % 10000) / (double)(10000 - 1);
    if (die <= EPSILON)
      bestMove = secondBestMove;
  }

  _DOM->makeMove(rep , side, bestMove); // make the chosen move (updates game state)

  if (verbose)
    printf("Examined %d nodes\n", numNodes);

  *termPercentage = (double)termCount / (double)numNodes; // compute percentage of nodes in tree that were terminal

  return bestMove;
}


/* Performs an alpha-beta search from the given board and returns the number of nodes visited */
int getAlphaBetaTreeSize(rep_t rep, int side, int depth, heuristics_t heuristic , int budget) {
  numNodes = termCount = 0; // reset node and terminal node counts
  alphaBeta(rep , 0, depth, side, MIN_WINS, MAX_WINS, heuristic, budget, id); // run specified alpha-beta search
  return numNodes; // return number of nodes expanded (updated by call to alphaBeta above)
}


/* Performs the specified alpha-beta search, while generating the structure of the search tree in dot format */
void genAlphaBetaTree(rep_t rep, int side, int depth, heuristics_t heuristic, int budget) {
  
  puts("digraph alphabeta {");
  puts("root = n1;");
  puts("node [shape=\"point\"];");
  puts("edge [arrowhead=\"none\"];");
  puts("n1 [shape=\"box\", style=\"filled\", color=\"blue\", height=0.2, width=0.2, label=\"\"];");

  // Generate the body of the graph using specified settings
  dotFormat = true;
  assert(id == 0); // for now, we only generate one graph per run
  alphaBeta(rep, 0, depth, side, MIN_WINS, MAX_WINS, heuristic, budget, ++id);
  dotFormat = false;

  printf("}\n");

  return;
}
