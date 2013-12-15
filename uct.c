#include "common.h"
#include "domain.h"


//The domain description TODO: have this as a singleton somewhere
extern DOM* DOM_; 
extern int debuglog;
extern int logIteration;
extern int log_this_iteration;

// File scope globals
static short dotFormat = false; // determines if search tree is printed out
static int id = 0; // used to determine next node id to assign
static int mm_Counter; // for counting nodes

/* This is a node in the UCT tree. */
typedef struct node {
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this node
  int n; // tracks the visit count
  int id; // used for graph visualization purposes
  int board[2][NUM_PITS+1]; // board position corresponding to this node
  int side; // side on move at this board position
  struct node** children; /* pointers to the children of this node -- note that index 0 remains
					unused (which is reserved for the store), so we have consistent move
					indexing/numbering */
} treeNode;


/* Routine to free up UCT tree */
static void freeTree(treeNode* node) {
  int i;
  for (i = 1; i < NUM_PITS+1; i++) {
    if (node->children[i]) {
      freeTree(node->children[i]);
      node->children[i] = NULL;
    }
  }

  free(node->children);
  free(node);
}
//TODO: 

/* Invoked by uctRecurse to decide which child of the current node should be expanded */
static int selectMove(treeNode* node, double C) {
  int i;
  double qhat;
  double score;
  int numBestMoves = 0;
  double bestScore;
  int bestMoves[NUM_PITS];
  // The multiplier is used to set the sign of the exploration bonus term (should be negative
  // for the min player and positive for the max player) i.e. so that we correctly compute
  // an upper confidence bound for Max and a lower confidence bound for Min
  double multiplier = (node->side == max) ? 1 : -1;

  for (i = 1; i < NUM_PITS+1; i++) { // iterate over all children
    if (node->board[node->side][i] == 0) // if the i^th move is illegal, skip it
      continue;

    // If the i^th child has never been visited before, select it first, before any other children are revisited
    if (node->children[i] == NULL)
      return i;

    // Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
    // children have been visited at least once)
    qhat = node->children[i]->scoreSum / (double)node->children[i]->n;  // exploitation component (this is the average utility)
    score = qhat + (multiplier * C) * sqrt(log(node->n) / (double)node->children[i]->n); // add exploration component

    // Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
    // is min's turn means we can always just take the maximum
    score = (node->side == min) ? -score : score;
    if(debuglog)printf("Score: %4.2f*(SQRT(LOG(%d)/%d)+=%4.2f/%d)=%4.2f\n",C,node->n,node->children[i]->n,node->children[i]->scoreSum,node->children[i]->n,score);
    // If this is either the first child, or the best scoring child, store it
    if ((numBestMoves == 0) || (score > bestScore)) {
      bestMoves[0] = i;
      bestScore = score;
      numBestMoves = 1;
//      printf("Best Score: %f\n",score);
    }
    else if (score == bestScore) // if this child ties with the best scoring child, store it
      bestMoves[numBestMoves++] = i;
      //printf("Added a best move: %d\n",i);
  }
  // Return the next child to explore (break ties randomly)
  //int chosenBestMove = bestMoves[random() % numBestMoves];
  int chosenBestMove = bestMoves[0];
  /*log*/
    if(logIteration){
        printf("%f: ",bestScore);
    }

  return chosenBestMove;
}


/* Recursively constructs the UCT search tree */
static double uctRecurse(treeNode* node, double C, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget, int backupOp) {
  double ret;
  int move;
  int i;
  double bestScore;
  double score;

  assert(node != NULL); // should never be calling uctRecurse on a non-existent node

  if ((ret = getGameStatus(node->board)) != INCOMPLETE) {
    // This is a terminal node (i.e. can't generate any more children)
    // If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
    // those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
    // which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
    // node values. If we are using engineered heuristics, then no rescaling is necessary.
    if ((heuristic == h3) || (heuristic == h4) || (heuristic == h5))
      ret /= MAX_WINS; // rescale

    if ((dotFormat) && (node->n == 0)) // on first visit to a terminal node, color it red
      printf("n%d [color=\"red\"];", node->id);

    // Update node score / count and return
    (node->n)++;
    node->scoreSum += ret;

    return ret;
  }
  else if (node->n == 0) { // not a terminal node
    // This is the first visit to this state -- we estimate its utility according to the user-specified
    // heuristic (playouts or other heuristics)
    ret = heuristic(node->board, node->side, budget);

    // Update node score / count and return
    (node->n)++;
    node->scoreSum += ret;

    return ret;
  }

  // We are at an internal node that has been visited before; descend recursively
  // Use selectMove to pick which branch to explore
  move = selectMove(node, C);

  // If this board does not have a node already created for it, create one for it now
  if (node->children[move] == NULL) {
    mm_Counter++;
    node->children[move] = calloc(1, sizeof(treeNode));
    //Alon
    node->children[move]->children = calloc(NUM_PITS+1, sizeof(treeNode*));
    cloneBoard(node->board, node->children[move]->board); // copy over the current board to child
    node->children[move]->side = node->side; // copy over the current side on move to child

    if (dotFormat) { // we are visiting a node for the first time -- assign it an id and print the edge leading to it
      node->children[move]->id = ++id;
      printf("n%d -> n%d;\n", node->id, id);
    }

    // From the current board, we make the move recommended by selectMove() -- the resulting game state
    // is what is stored in the child node
    makeMove(node->children[move]->board, &(node->children[move]->side), move);
    if(debuglog)printf("Created a node for move %d\n",move);
  }

    /*log*/
    if(logIteration){
        printBoard(node->children[move]->board,node->children[move]->side);
    }
    /*log*/
  // Descend recursively
  ret = uctRecurse(node->children[move], C, heuristic, budget, backupOp);

  // Update score and node counts and return the outcome of this episode
  if (backupOp == AVERAGE) { // use averaging back-up
    (node->n)++;
    node->scoreSum += ret;
  }
  else if (backupOp == MINMAX) { // use minimaxing back-up
    (node->n)++;
    bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;

    for (i = 1; i < NUM_PITS+1; i++) {
      if (node->children[i]) { // if child exists, is it the best scoring child?
	score = node->children[i]->scoreSum / (double)node->children[i]->n;
	if (   ((node->side == max) && (score > bestScore))
	    || ((node->side == min) && (score < bestScore)))
	  bestScore = score;
      }
    }

    node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
  }
  else { // shouldn't happen
    puts("Invalid back-up operator!");
    exit(1);
  }

  return ret;
}


/* Takes current board and side on move and runs numIterations of UCT with exploration
   bias of C. The outcome of the search is then used to make the best move (which is
   the value returned) */
int makeUCTMove(int board[2][NUM_PITS+1], int *side, int numIterations, double C,
		double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget,
		int* bestMoves, int* numBestMoves, int backupOp) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;

  *numBestMoves = 0; // reset size of set of best moves

  if(debuglog)puts("UCT");
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  cloneBoard(board, rootNode->board);
  rootNode->side = *side;
  //Alon
  rootNode->children = calloc(NUM_PITS+1, sizeof(treeNode*));

  // Run specified number of iterations of UCT
  for (i = 0; i < numIterations; i++){
      if(debuglog)printf("UCT:Iteration: %d\n",i);
      if(i==log_this_iteration){ //3224
        debuglog=true;
        logIteration=true;
    }
    else {
        debuglog=false;
        logIteration=false;
    }
    uctRecurse(rootNode, C, heuristic, budget, backupOp);
  }

  // Now look at the children 1-ply deep and determing the best one (break ties
  // randomly)
  for (i = 1; i < NUM_PITS+1; i++) { // for each move
    if (board[*side][i] == 0) // skip illegal moves
      continue;

    if (!rootNode->children[i]) // this node was not created since # iterations was too small
      continue;

    // Compute average utility of this child
    val = rootNode->children[i]->scoreSum / (double)rootNode->children[i]->n;

    // If this was min's move, negate the utility value (this makes things a little cleaner
    // as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
    val = (*side == min) ? -val : val;

    // If this is the first child, or the best scoring child, then store it
    if ((*numBestMoves == 0) || (val > bestScore)) {
      bestMoves[0] = i;
      bestScore = val;
      *numBestMoves = 1;
    }
    else if (val == bestScore) // child ties with currently best scoring one; store it
      bestMoves[(*numBestMoves)++] = i;

    if (verbose)
      printf("Move # %d -- Value %f, Count %d\n", i, ((*side == min) ? -val : val), rootNode->children[i]->n);
  }

  // We should have at least looked at one child
  assert(*numBestMoves != 0);

  bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
  makeMove(board, side, bestMove); // make it (updates game state)

  if (verbose) {
    printf("Value of root node: %f\n", rootNode->scoreSum / (double)rootNode->n);
    printf("Best move: %d\n", bestMove);
  }

  // Clean up before returning
  freeTree(rootNode);

  return bestMove;
}


/* Performs the specified UCT search, while generating the structure of the search tree in dot format */
void genUCTTree(int board[2][NUM_PITS+1], int side, int numIterations, double C, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget) {
  treeNode* rootNode;
  int i;

  puts("digraph uct {");
  puts("root = n1;");
  puts("node [shape=\"point\"];");
  puts("edge [arrowhead=\"none\"];");
  puts("n1 [shape=\"box\", style=\"filled\", color=\"blue\", height=0.2, width=0.2, label=\"\"];");

  // Generate the body of the graph using specified settings
  dotFormat = true;
  assert(id == 0);  // for now, we only generate one graph per run

  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  cloneBoard(board, rootNode->board);
  rootNode->side = side;
  rootNode->id = ++id;
  //Alon
  rootNode->children = calloc(NUM_PITS+1, sizeof(treeNode*));

  // Run specified number of iterations of UCT (this outputs the search tree)
  for (i = 0; i < numIterations; i++)
    uctRecurse(rootNode, C, heuristic, budget, AVERAGE);

  dotFormat = false;

  printf("}\n");

  // Clean up
  freeTree(rootNode);

  return;
}


/* Recursively computes the minimax value of the UCT-constructed tree rooted at 'node' */
static double minmaxUCT(treeNode* node) {
  int i;
  double val;
  double bestScore;

  // Should never be reaching a non-existent node
  assert(node != NULL);

  // Initialize bestScore to a very unfavorable value based on who is on move
  bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;

  // Is this a terminal node?
  if ((val = getGameStatus(node->board)) != INCOMPLETE)
    return val; // return something from the set {MIN_WINS, DRAW, MAX_WINS}

  // Is this a leaf node? (can determine this by looking at the UCT visit count to this node)
  if (node->n == 1) {
    assert((node->scoreSum > MIN_WINS) && (node->scoreSum < MAX_WINS)); // make sure heuristic is bounded by terminal node values
    return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
  }

  // Otherwise, we are at an internal node, so descend recursively
  for (i = 1; i < NUM_PITS+1; i++) {
    if (node->children[i]) { // only descend if child exists
      val = minmaxUCT(node->children[i]); // compute minimax value of i^th child
      if ((node->side == max) && (val > bestScore)) // maximizing level -- is this child score higher than the best so far?
	bestScore = val;
      else if ((node->side == min) && (val < bestScore)) // minimizing level -- is this child score lower than the best so far?
	bestScore = val;
    }
  }

  return bestScore;
}


/* Runs specified number of iteration of UCT. Then, runs minimax on the resulting UCT tree and returns the best move */
int makeMinmaxOnUCTMove(int board[2][NUM_PITS+1], int *side, int numIterations, double C,
			double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget,
			int* bestMoves, int* numBestMoves) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;

  *numBestMoves = 0; // reset size of set of best moves

  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = (treeNode*)calloc(1, sizeof(treeNode));
  cloneBoard(board, rootNode->board);
  rootNode->side = *side;
  //Alon
  rootNode->children = calloc(NUM_PITS+1, sizeof(treeNode*));

  // Run specified number of iterations of UCT
  for (i = 0; i < numIterations; i++)
    uctRecurse(rootNode, C, heuristic, budget, AVERAGE);

  // Now minimax the tree we just built (we minimax starting from each child)
  for (i = 1; i < NUM_PITS+1; i++) {
    if (rootNode->children[i]) { // if this child was explored
      val = minmaxUCT(rootNode->children[i]); // do a minmax backup of the subtree rooted at this child

      // If this was min's move, negate the utility value (this makes things a little cleaner
      // as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
      val = (*side == min) ? -val : val;

      // If this is the first child, or the best scoring child, then store it
      if ((*numBestMoves == 0) || (val > bestScore)) {
	bestMoves[0] = i;
	bestScore = val;
	*numBestMoves = 1;
      }
      else if (val == bestScore) // child ties with currently best scoring one; store it
	bestMoves[(*numBestMoves)++] = i;

      if (verbose)
	printf("Move # %d -- Value %f\n", i, ((*side == min) ? -val : val));
    }
  }

  // We should have at least looked at one child
  assert(*numBestMoves != 0);

  bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
  makeMove(board, side, bestMove); // make it (updates game state)

  if (verbose)
    printf("Best move: %d\n", bestMove);

  // Clean up when done
  freeTree(rootNode);

  return bestMove;
}

void printUctStats(){
    printf("** Total UCT nodes %d**\n",mm_Counter);
}
