
#include "common.h"

/* Routines related to a random player */

/* Given a board and side on move, returns a legal move chosen uniformly at random */
int pickRandomMove(int board[2][NUM_PITS+1], int side) {
  int i;
  int numMoves = 0;
  int moves[NUM_PITS];

  for (i = 1; i < NUM_PITS+1; i++)
    if (board[side][i] > 0) // only legal moves are considered
      moves[numMoves++] = i;

  return moves[random() % numMoves];
}


/* Given a board and side on move, makes a move uniformly at random (game state is updated) */
int makeRandomMove(int board[2][NUM_PITS+1], int *side) {
  int move;

  move = pickRandomMove(board, *side);
  makeMove(board, side, move);

  return move;
}
