#ifndef CHESS_H_INCLUDED
#define CHESS_H_INCLUDED
#include "common.h"



/*
 implementation of the getNumOfChildren
*/
int getNumOfChildren_chess();

/*

*/
int estimateTreeSize_chess(int treeSize);


/*
*
*/
int isValidChild_chess(rep_t rep, int side, int move);

/*
*
*/

int getGameStatus_chess(rep_t rep);


/*
**/
void makeMove_chess(rep_t rep,int * side, int move);
/*
*
**/
rep_t cloneRep_chess(rep_t orig);



/*
* Write random state to rep
*/
void generateRandomStart_chess(rep_t rep,int side);

/**
 copy
**/
void copy_chess (rep_t src,rep_t dst);


/*
  Allocate a new chess board
*/
rep_t allocate_chess();
/*
* Destruct the board
*/
void destructRep_chess(rep_t rep);
/*
*
*/
double applyHeuristics_chess (heuristics_t h,rep_t rep,int side, int budget);

#endif // CHESS_H_INCLUDED
