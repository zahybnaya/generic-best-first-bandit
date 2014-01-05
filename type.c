#include "type.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static int mysteryNodes = 0;

static void destroy_mmOracle(void *void_ts) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  for (i = 0; i < ts->numTypes; i++) {
    free(((type_mmOracle *)ts->types[i])->openList);
    free(ts->types[i]);
  }
  
  free(ts->types);

  //Clode and free the minmax lookup file
  close(*(int *)(ts->extra));
  remove(ORACLE_PATH);
  free(ts->extra);
}

static treeNode *selectFromType_mmOracle(void *void_t, double C) {
  int i;
  type_mmOracle *t = (type_mmOracle *)void_t;
  
  //Choose lowest depth node
  treeNode *node;
  int min_depth = INF;
  int min_node;
  for (i = 0; i < t->tail; i++) {
    if (t->openList[i] && t->openList[i]->depth < min_depth) {
      min_node = i;
      min_depth = t->openList[i]->depth;
    }
  }
  node = t->openList[min_node];
  t->openList[min_node] = 0;
  t->size--;
  t->empty = min_node;
  
  return node;
}

//Treat the board as a number in base #ofStones, convert it to decimal and use it as an offset
static int mancalaStateToOffset(rep_t rep) {
  int i, j, k;
  int offset = 0;
  int baseTothePowerOfIndex;

  for (i = 0; i < 1; i++) {
    for (j = 0; j < _DOM->getNumOfChildren(); j++) {
      baseTothePowerOfIndex = 1;

      for (k = _DOM->getNumOfChildren() - 1 - j; k > 0; k--)
	baseTothePowerOfIndex = baseTothePowerOfIndex * MAX_WINS;
      
      offset = offset + baseTothePowerOfIndex * ((int**)rep)[i][j];
    }
  }
  
  return offset;
}

//Save in a file the minimax value of the given board
static void saveMMVal(int fd, rep_t rep, int val) {
  //To differentiate an empty byte and a minimax value of zero, use a number out of the range
  //to denote zero
  if (val == 0)
    val = MAX_WINS + 1;
 
  lseek(fd, mancalaStateToOffset(rep), SEEK_SET);
  write(fd, &val, 1);
}

//Opposite of saveMMVal
//Return -1 if no mm value found
static int readMMVal(int fd, rep_t rep) {
  char val; //char so the correct signed value will be read
  
  lseek(fd, mancalaStateToOffset(rep), SEEK_SET);
  read(fd, &val, 1);

  //NO mm value
  if (val == 0)
    val = -1;
  
  //Shift back special value for zero
  if (val == MAX_WINS + 1)
    val = 0;
  
  return val;
}

//Run minimax without pruning and store the minimax value of every traversed node in FD.
static double storeMinimax(int fd, rep_t rep, int searchDepth, int depth, int side, heuristics_t heuristic, int budget) {
  int val1, val2;
  int i;
  rep_t dummyRep;
  int origSide = side;  
  
  // Hit terminal node -- return value is one of {MAX_WINS, MIN_WINS, DRAW}
  if ((val1 = _DOM->getGameStatus(rep)) != INCOMPLETE) {
    saveMMVal(fd, rep, val1);
    return val1;
  }

  // Hit search depth cutoff -- apply the chosen heuristic function to this node and return that value
  if (searchDepth >= depth) {

    val1 = heuristic(rep, side, budget);

    // Ensure that the heuristic value is always bounded by the values assigned to terminal nodes (i.e. true win/loss
    // positions)
    assert ((val1 < MAX_WINS) && (val1 > MIN_WINS));
    saveMMVal(fd, rep, val1);
    return val1;
  }

  // Otherwise, recurse
  if (side == max) { // at a Max node
    val1 = -INF;
    for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i)) // skip illegal moves
	continue;
      dummyRep = _DOM->cloneRep(rep); // clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i); // generate i^th child

      val2 = storeMinimax(fd, dummyRep, searchDepth+1, depth, side, heuristic, budget); // compute minimax value of this child
      val1 = (val2 > val1) ? val2 : val1; // choose maximal child
    }
  }
  else { // at a Min node
    val1 = INF;
    for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all possible moves
      side = origSide;
      if (!_DOM->isValidChild(rep,side,i)) // skip illegal moves
	continue;
      dummyRep = _DOM->cloneRep(rep); // clone starting board so it can be restored on next pass through loop
      _DOM->makeMove(dummyRep,&side,i); // generate i^th child

      val2 = storeMinimax(fd, dummyRep, searchDepth+1, depth, side, heuristic, budget); // compute minimax value of this child
      val1 = (val2 < val1) ? val2 : val1; //choose minimal child
    }
  }
  
  saveMMVal(fd, rep, val1);
  return val1;
}

//Calculate the type of a node and place it within its open list, allocating more space as needed.
static void assignToType_mmOracle(void *void_ts, treeNode *node, int fatherType, int threshold) {
  type_system *ts = (type_system *)void_ts;
  int mmVal = readMMVal(*(int *)ts->extra, node->rep);
  
  //Found a node outside of the range of mm values the oracle computed
  if (mmVal == -1) {
    mmVal = ORACLE_MISSING_H(node->rep, node->side);
    mysteryNodes++;
  }

  mmVal = mmVal + MAX_WINS; //Shift possible mm values to natural numbers so they can be used as an index into the type system.

  type_mmOracle *t = (type_mmOracle *)ts->types[mmVal];

  //if the open list of this type is full,
  //allocate a new open list for it and copy the new one into it.
  if (t->tail == t->capacity) {
    t->capacity = 2 * (t->capacity + 1);
    t->openList = realloc(t->openList, t->capacity * sizeof(treeNode *));
  }
  
  if (t->empty != -1) {
    t->openList[t->empty] = node;
    t->empty = -1;
  } else {
    t->openList[t->tail] = node;
    t->tail++;
  }
  t->size++;
}

void furtherInit_mmOracle(void *void_ts, rep_t rep, int side) {
  int i;
  type_system *ts = (type_system *)void_ts;
  
  ts->numTypes = MAX_WINS * 2 + 1; //Number of possible minmax values
  ts->types = calloc(ts->numTypes, sizeof(type_mmOracle *));
      
  for (i = 0; i < ts->numTypes; i++) {
     ts->types[i] = calloc(1, sizeof(type_mmOracle));
     ((type_mmOracle *)ts->types[i])->empty = -1;
  }
      
  ts->extra = calloc(1, sizeof(int));
  *(int *)(ts->extra) = open(ORACLE_PATH, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  storeMinimax(*(int *)(ts->extra), rep, 0, ORACLE_DEPTH, side, ORACLE_H, 0); 
}

void furtherInit_sts(void *void_ts, rep_t rep, int side) {
  type_system *ts = (type_system *)void_ts;
  
  ts->numTypes = 1;
  ts->types = calloc(ts->numTypes, sizeof(type_sts *));
  ts->types[0] = calloc(1, sizeof(type_sts));
}

static void assignToType_sts(void *void_ts, treeNode *node, int fatherType, int threshold) {
  type_system *ts = (type_system *)void_ts;
  int i;
  
  if (fatherType == -1) //no father means root node
    ((type_sts *)ts->types[0])->root = node;
  else {
    type_sts *t = (type_sts *)ts->types[fatherType];
    
    //The size of a type is the size of the subtree rooted at the node representing the type
    //which is updated at the expansion phase, so only need to take care of splitting types
    if (t->root->subtreeSize > threshold) {
      //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
      //is the ancestor of the current node to be inserted.
      //This ancestor will become the root of a new type which will be in the same index as the old type.
      treeNode *newFather = node;
      while (newFather->parent != t->root)
	newFather = newFather->parent;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	if (!_DOM->isValidChild(t->root->rep, t->root->side, i) || t->root->children[i] == newFather) //skip empty children or this is the new father (will be taken care of later)
	  continue;
	
	ts->numTypes++;
	ts->types = realloc(ts->types, ts->numTypes * sizeof(type_sts *)); //alocate a new type
	
	ts->types[ts->numTypes - 1] = calloc(1, sizeof(type_sts));
	((type_sts *)ts->types[ts->numTypes - 1])->root = t->root->children[i];
	ts->types[ts->numTypes - 1]->visits = t->root->children[i]->n;
      }
      
      t->root = node;
      t->visits = node->n;
    }
  } 
}

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

static treeNode *selectFromType_sts(void *void_t, double C) {
  type_sts *t = (type_sts *)void_t;
  treeNode *node = t->root;
  
  //travel down the tree using ucb
  while (_DOM->getGameStatus(node->rep) == INCOMPLETE && node->n > 0)
    node = node->children[selectMove(node, C)];
 
  return node;
}

static void destroy_sts(void *void_ts) {
  type_system *ts = (type_system *)void_ts;
  free(ts->types);
}

void *init_type_system(int t) {
  type_system *ts = calloc(1, sizeof(type_system));
  
  switch (t) {
    case MM_ORACLE:      
      ts->furtherInit = furtherInit_mmOracle;
      ts->assignToType = assignToType_mmOracle;
      ts->selectFromType = selectFromType_mmOracle;
      ts->destroy = destroy_mmOracle;
      break;
    case STS:
      ts->furtherInit = furtherInit_sts;
      ts->assignToType = assignToType_sts;
      ts->selectFromType = selectFromType_sts;
      ts->destroy = destroy_sts;
      break;
  }
  return ts;
}