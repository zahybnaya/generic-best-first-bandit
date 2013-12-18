#include "common.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ORACLE_DEPTH 12

//Treat the board as a number in base #ofStones, convert it to decimal and use it as an offset
//in the file
static void saveMMVal(int fd, int board[2][NUM_PITS + 1], int val) {
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
 
  lseek(fd, offset, SEEK_SET);
  
  //To differentiate an empty byte and a minimax value of zero, use a number out of the range
  //to denote zero
  if (val == 0) {
    val = MAX_WINS + 1;
    write(fd, &val, 1);
  } else
    write(fd, &val, 1);
  printf("offset %d val %d\n" ,offset, val);
}

//Run minimax without pruning and store the minimax value of every traversed node in FD.
static double storeMinimax(int board[2][NUM_PITS+1], int searchDepth, int depth, int side, double (*heuristic)(int board[2][NUM_PITS+1],int,int), 
			    int budget, int fd) {
  int val1, val2;
  int i;
  int dummyBoard[2][NUM_PITS+1];
  int origSide = side;  
  
  // Hit terminal node -- return value is one of {MAX_WINS, MIN_WINS, DRAW}
  if ((val1 = getGameStatus(board)) != INCOMPLETE) {
    saveMMVal(fd, board, val1);
    return val1;
  }

  // Hit search depth cutoff -- apply the chosen heuristic function to this node and return that value
  if (searchDepth >= depth) {

    val1 = heuristic(board, side, budget);

    // Ensure that the heuristic value is always bounded by the values assigned to terminal nodes (i.e. true win/loss
    // positions)
    assert ((val1 < MAX_WINS) && (val1 > MIN_WINS));
    saveMMVal(fd, board, val1);
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

      val2 = storeMinimax(dummyBoard, searchDepth+1, depth, side, heuristic, budget, fd); // compute minimax value of this child
      val1 = (val2 > val1) ? val2 : val1; // we have tightened our alpha bound
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

      val2 = storeMinimax(dummyBoard, searchDepth+1, depth, side, heuristic, budget, fd); // compute minimax value of this child
      val1 = (val2 < val1) ? val2 : val1; // we have tightened our beta bound
    }
  }
  
  saveMMVal(fd, board, val1);
  return val1;
}

void preComputeMM(int board[2][NUM_PITS+1], int side, double (*heuristic)(int board[2][NUM_PITS+1],int,int), int budget) {
  char *path = "MMValues.txt";
  int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);

  storeMinimax(board, 0, ORACLE_DEPTH, side, heuristic, budget, fd);
  
  close(fd);
}