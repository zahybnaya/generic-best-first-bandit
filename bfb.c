#include "common.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ORACLE_DEPTH 12

static int oraclefd;
static int mysteryNodes = 0;

/* This is a node in the UCT tree. */
typedef struct node {
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this node
  int n; // tracks the visit count
  int id; // used for graph visualization purposes
  int board[2][NUM_PITS+1]; // board position corresponding to this node
  int side; // side on move at this board position
  struct node *parent; //the parent node
  struct node** children; /* pointers to the children of this node -- note that index 0 remains
					unused (which is reserved for the store), so we have consistent move
					indexing/numbering */
} treeNode;

struct type {
  //UCB stats
  double scoreSum; // stores the sum of the rewards of the episodes that have gone through this type
  int visits; // tracks the visit count
  
  //Type stats
  treeNode **openList; //tree nodes which havn't been fully expanded and are of the same type
  int capacity; //Maximum current capacity of the open list.
  int tail;  //The first empty index in the open list.
  int empty; //If a node was extracted from the open list than this will be its index until filled again, else -1.
};

static struct type **type_system;
static int numTypes; //number of types currently in the type system

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

/* Routine to free up type system */
static void freeTypeSystem() {
  int i;
  
  for (i = 0; i < numTypes; i++) {
    free(type_system[i]->openList);
    free(type_system[i]);
  }
  
  free(type_system);
}

//Treat the board as a number in base #ofStones, convert it to decimal and use it as an offset
static int mancalaStateToOffset(int board[2][NUM_PITS + 1]) {
  int i, j, k;
  int offset = 0;
  int baseTothePowerOfIndex;

  for (i = 0; i < 1; i++) {
    for (j = 0; j < NUM_PITS + 1; j++) {
      baseTothePowerOfIndex = 1;

      for (k = NUM_PITS - j; k > 0; k--)
	baseTothePowerOfIndex = baseTothePowerOfIndex * MAX_WINS;
      
      offset = offset + baseTothePowerOfIndex * board[i][j];
    }
  }
  
  return offset;
}

//Save in a file the minimax value of the given board
static void saveMMVal(int board[2][NUM_PITS + 1], int val) {
  //To differentiate an empty byte and a minimax value of zero, use a number out of the range
  //to denote zero
  if (val == 0)
    val = MAX_WINS + 1;
 
  lseek(oraclefd, mancalaStateToOffset(board), SEEK_SET);
  write(oraclefd, &val, 1);
}

//Opposite of saveMMVal
//Return -1 if no mm value found
static int readMMVal(int board[2][NUM_PITS + 1]) {
  int val;
  
  lseek(oraclefd, mancalaStateToOffset(board), SEEK_SET);
  read(oraclefd, &val, 1);
  
  //NO mm value
  if (val == 0)
    val = -1;
  
  //Shift back special value for zero
  if (val == MAX_WINS + 1)
    val = 0;
  
  return val;
}

//Run minimax without pruning and store the minimax value of every traversed node in FD.
static double storeMinimax(int board[2][NUM_PITS+1], int searchDepth, int depth, int side, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget) {
  int val1, val2;
  int i;
  int dummyBoard[2][NUM_PITS+1];
  int origSide = side;  
  
  // Hit terminal node -- return value is one of {MAX_WINS, MIN_WINS, DRAW}
  if ((val1 = getGameStatus(board)) != INCOMPLETE) {
    saveMMVal(board, val1);
    return val1;
  }

  // Hit search depth cutoff -- apply the chosen heuristic function to this node and return that value
  if (searchDepth >= depth) {

    val1 = heuristic(board, side, budget);

    // Ensure that the heuristic value is always bounded by the values assigned to terminal nodes (i.e. true win/loss
    // positions)
    assert ((val1 < MAX_WINS) && (val1 > MIN_WINS));
    saveMMVal(board, val1);
    return val1;
  }

  // Otherwise, recurse
  if (side == max) { // at a Max node
    val1 = -INF;
    for (i = 1; i < NUM_PITS+1; i++) { // iterate over all possible moves
      side = origSide;
      if (board[side][i] == 0) // skip illegal moves
	continue;
      cloneBoard(board, dummyBoard); // clone starting board so it can be restored on next pass through loop
      makeMove(dummyBoard, &side, i); // generate i^th child

      val2 = storeMinimax(dummyBoard, searchDepth+1, depth, side, heuristic, budget); // compute minimax value of this child
      val1 = (val2 > val1) ? val2 : val1; // choose maximal child
    }
  }
  else { // at a Min node
    val1 = INF;
    for (i = 1; i < NUM_PITS+1; i++) { // iterate over all possible moves
      side = origSide;
      if (board[side][i] == 0) // skip illegal moves
	continue;
      cloneBoard(board, dummyBoard); // clone starting board so it can be restored on next pass through loop
      makeMove(dummyBoard, &side, i); // generate i^th child

      val2 = storeMinimax(dummyBoard, searchDepth+1, depth, side, heuristic, budget); // compute minimax value of this child
      val1 = (val2 < val1) ? val2 : val1; //choose minimal child
    }
  }
  
  saveMMVal(board, val1);
  return val1;
}

//Calculate the type of a node and place it within its open list, allocating more space as needed.
static void assignToType(treeNode *node, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int side, int budget) {
  int mmVal = readMMVal(node->board);
  
  //Found a node outside of the range of mm values the oracle computed
  if (mmVal == -1) {
    mmVal = heuristic(node->board, side, budget);
    mysteryNodes++;
  }

  mmVal = mmVal + MAX_WINS; //Shift possible mm values to natural numbers so they can be used as an index into the type system.
  
  struct type *t = type_system[mmVal];
  
  //if the open list of this type is full,
  //allocate a new open list for it and copy the new one into it.
  if (t->tail == t->capacity) {
    t->capacity = 2 * (t->capacity + 1); //TODO maybe init types before hand and set capacity
    t->openList = realloc(t->openList, t->capacity * sizeof(treeNode *));
  }
  
  if (t->empty != -1) {
    t->openList[t->empty] = node;
    t->empty = -1;
  } else {
    t->openList[t->tail] = node;
    t->tail++;
  }
}

/* Invoked by bfbIteration to decide which type of node should be expanded */
static int selectType(double C, int visits) {
  int i;
  double qhat;
  double score;
  int numBestTypes = 0;
  double bestScore;
  int bestTypes[numTypes];

  for (i = 0; i < numTypes; i++) { // iterate over all types
    if (type_system[i]->tail == 0) // if no nodes in type continue
      continue;

    //If the type has never been visited before, select it first
    if (type_system[i]->visits == 0)
      return i;

    // Otherwise, compute this type's UCB1 index (will be used to pick best type if it transpires that all
    // types have been visited at least once)
    qhat = type_system[i]->scoreSum / (double)type_system[i]->visits;  // exploitation component (this is the average utility)
    score = qhat + C * sqrt(log(visits) / (double)type_system[i]->visits); // add exploration component

    // If this is either the first type, or the best scoring type, store it
    if ((numBestTypes == 0) || (score > bestScore)) {
      bestTypes[0] = i;
      bestScore = score;
      numBestTypes = 1;
    }
    else if (score == bestScore) // if this child ties with the best scoring type, store it
      bestTypes[numBestTypes++] = i;
    
  }

  // Return the next type to explore (break ties randomly)
  printf("random type %d\n", random() % numBestTypes); //TODO delete
  return bestTypes[random() % numBestTypes];
}

//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(int visits, double C, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget, int backupOp) {  
  //Choose type that maximizes UCB1
  struct type *t = type_system[selectType(C, visits)];
 
  //Choose random node from type, take it out, update type->empty
  int randomIndex = random() % t->tail;
  treeNode *node = t->openList[randomIndex];
  t->empty = randomIndex;
  
  //rollout
  int ret = heuristic(node->board, node->side, budget);
  
  //update type ucb stats and backpropagate
  t->visits++;
  t->scoreSum = t->scoreSum + ret;
  treeNode *bp = node;
  while (bp != NULL) {
    bp->n++;
    bp->scoreSum = bp->scoreSum + ret;
    bp = bp->parent;
  }
  
  //generate the children and place them in the type system
  int i;
  for (i = 1; i < NUM_PITS + 1; i++) {
    if (node->board[node->side][i] == 0) // if the i^th move is illegal, skip it
      continue;
    
    node->children[i] = calloc(1, sizeof(treeNode));
    node->children[i]->children = calloc(NUM_PITS + 1, sizeof(treeNode*));
    cloneBoard(node->board, node->children[i]->board); // copy over the current board to child
    node->children[i]->side = node->side; // copy over the current side on move to child
    makeMove(node->children[i]->board, &(node->children[i]->side), i); //Make the i-th move
    node->children[i]->parent = node; //save parent
    
    assignToType(node->children[i], heuristic, node->children[i]->side, budget);
  }
}

int makeBFBMove(int board[2][NUM_PITS+1], int *side, int numIterations, double C,
		double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget,
		int* bestMoves, int* numBestMoves, int backupOp) {
  int i;
  double val;
  int bestMove = NULL_MOVE;
  double bestScore;
  treeNode* rootNode;

  *numBestMoves = 0; // reset size of set of best moves

  //Oracle - TODO remove or parametrize when done with the POC
  oraclefd = open("MMValues.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  storeMinimax(board, 0, ORACLE_DEPTH, *side, h2, 0); 
  //Init type system to the maximum possible number of types. TODO will need to change this for other typs... TODO put somewhere else
  numTypes = MAX_WINS * 2;
  type_system = calloc(numTypes, sizeof(struct type *));
  for (i = 0; i < numTypes; i++) {
    type_system[i] = calloc(1, sizeof(struct type));
    type_system[i]->empty = -1;
  }
  
  // Create the root node of the UCT tree; populate the board and side on move fields
  rootNode = calloc(1, sizeof(treeNode));
  rootNode->children = calloc(NUM_PITS+1, sizeof(treeNode*));
  cloneBoard(board, rootNode->board);
  rootNode->side = *side;

  //assign the root to a type
  assignToType(rootNode, heuristic, *side, budget);

  // Run specified number of iterations of BFB
  for (i = 0; i < 1; i++)
    bfbIteration(i, C, heuristic, budget, backupOp);

  close(oraclefd); //TODO: do something with this when done with POC

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
  freeTypeSystem();
  
  return bestMove;
}