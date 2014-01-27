#ifndef CONNECT4_H__
#define CONNECT4_H__

#include "common.h"

#define BOARD_ROWS 6
#define BOARD_COLS 7

int getNumOfChildren_connect4();
int estimateTreeSize_connect4(int treeSize);
int isValidChild_connect4(rep_t rep, int side, int move);
int getGameStatus_connect4(rep_t rep);
void makeMove_connect4(rep_t rep,int * side, int move);
rep_t cloneRep_connect4(rep_t orig);
double applyHeuristics_connect4 (heuristics_t h,rep_t rep,int side, int budget);
void generateRandomStart_connect4(rep_t rep,int*);
rep_t allocate_connect4();
void destructRep_connect4(rep_t rep);
void copy_connect4(rep_t src,rep_t dst);

double h3_c4(rep_t rep, int side, int numPlayouts);
static int playout_c4(int * board, int side);
int pickRandomMove_connect4(int * dummyBoard, int side);

int checkWin(int *board, int *side);
int checkFour(int *board, int a, int b, int c, int d);
int horizontalCheck(int *board, int *side);
int verticalCheck(int *board, int *side);
int diagonalCheck(int *board, int *side);
void printBoard_c4(int *board);

#endif
