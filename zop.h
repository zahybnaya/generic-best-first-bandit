#ifndef __CHECKERS__H
#define  __CHECKERS__H
#include "domain.h"

/***
 * Zop domain (Zoog or pered "even or odd"). 
 *
 * Basically this is just a mock domain to test the generic code.
 * 
 * Two players. 
 * Max player: always wants a an even number 
 * Min Player: always wants a an odd number 
 *
 *
 *
 * */

int getNumOfChildren_zop();
int estimatedTreeSize_zop(int treeSize);
int isValidChild_zop(rep_t rep, int side,int move);
int getGameStatus_zop(rep_t rep);
void makeMove_zop(rep_t rep,int * side, int move);
rep_t cloneRep_zop(rep_t orig);
double applyHeuristics_zop(heuristics_t h,rep_t rep,int side, int budget);
void destructRep_zop (rep_t rep);
rep_t allocate_zop();
void generateRandomStart_zop(rep_t state, int side);
void copy_zop (rep_t src ,rep_t dest);

double h1_zop(rep_t , int, int);


#endif
