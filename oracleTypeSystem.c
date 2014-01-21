#include "type.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static int mysteryNodes = 0;

void destroy_mmOracle(void *void_ts) {
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

treeNode *selectFromType_mmOracle(void *void_t, double C) {
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
double storeMinimax(int fd, rep_t rep, int searchDepth, int depth, int side, heuristics_t heuristic, int budget) {
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
void assignToType_mmOracle(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
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