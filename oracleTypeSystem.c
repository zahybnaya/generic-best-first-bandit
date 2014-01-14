/***
 *
 * These are the oracle type system functions 
 *
 */
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
  char val; //char so the correct signed value will be read
  
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
  
  /*t->openList[t->tail] = node;
  t->tail++;
  t->size++;*/
  if (t->empty != -1) {
    t->openList[t->empty] = node;
    t->empty = -1;
  } else {
    t->openList[t->tail] = node;
    t->tail++;
  }
  t->size++;
  
  if (t->size == 1) {
    
  }
  
}


