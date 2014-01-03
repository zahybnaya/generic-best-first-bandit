
#include "common.h"

/* Contains two generic routines central to Mancala game-play */

/* Make specified move and update game state (a move is just a pit index on the board) */
void makeMove(int board[2][NUM_PITS+1], int* side, int move) {
  int sideToAdd = *side;
  int currentPit;
  int numShells = board[*side][move];

  // Empty selected pit
  board[*side][move] = 0;
  currentPit = move;

  // Place stones counter-clockwise, skipping opponent's store
  while (numShells > 0) {
    currentPit--;

    // Handle wrap-around / skipping opponent's store
    if ((currentPit < 0) || ((currentPit == store) && (sideToAdd != *side))) {
      sideToAdd = 1^sideToAdd;
      currentPit = NUM_PITS;
    }

    board[sideToAdd][currentPit]++;
    numShells--;
  }

  // If we end on a pit on our side that was previously empty, immediately move the stone to
  // the store, as well as any stones on the opponent's side of the board, across from this pit
  if ((sideToAdd == *side) && (board[sideToAdd][currentPit] == 1) && (currentPit != store)) {
    board[*side][store] += board[1^*side][NUM_PITS-currentPit+1];
    board[1^*side][NUM_PITS-currentPit+1] = 0;
    board[*side][store]++;
    board[*side][currentPit] = 0;
  }

  // Current side gets to go again if the turn ends with placing a stone in one's own store
  if ((sideToAdd != *side) || (currentPit != store))
    *side = 1^*side;
    
  return; 
}


/* Given a board and side on move, compute the number of legal moves available */
int getNumLegalMoves(int board[2][NUM_PITS+1], int side) {
  int numLegal = 0;
  int i;

  for (i = 1; i < NUM_PITS+1; i++)
    if (board[side][i] != 0)
      numLegal++;
  
  return numLegal;
}


/* Returns the current status of the board -- a value from the set {MAX_WINS, DRAW, MIN_WINS,
   INCOMPLETE} */
int getGameStatus(int **board) {
  int i, j;
  int gameOver = true;
  int maxScore, minScore;

  for (i = max; i <= min; i++) {
    gameOver = true;

    // First check if the game is over -- for this to happen, one of the two sides must have
    // no legal moves left (i.e. all pits must be empty)
    for (j = 1; j < NUM_PITS+1; j++) {
      if (board[i][j] != 0) {
	gameOver = false;
	break;
      }
    }

    // If the game is indeed over, figure out who won. 
    // Each side's score = # stones in own store + # stones on opponent's side of board not in store
    // The winner is the one with the higher score
    if (gameOver) {
      maxScore = board[max][store];
      minScore = board[min][store];
      for (j = 1; j < NUM_PITS+1; j++) {
	maxScore += board[min][j];
	minScore += board[max][j];
      }
      if (maxScore > minScore)
	return MAX_WINS;
      else if (minScore > maxScore)
	return MIN_WINS;
      else
	return DRAW;
    }
  }

  // Both sides have legal moves left, so game is incomplete
  return INCOMPLETE;
}
