#ifndef GGP_H_INCLUDED
#define GGP_H_INCLUDED

#include "domain.h"

int getNumOfChildren_ggp(rep_t rep, int side);
int isValidChild_ggp(rep_t rep, int side, int move);
int getGameStatus_ggp(rep_t rep);
void makeMove_ggp(rep_t rep,int * side, int move);
rep_t cloneRep_ggp(rep_t orig);
void generateRandomStart_ggp(rep_t rep,int*);
rep_t allocate_ggp();
void destructRep_ggp(rep_t rep);
void copy_ggp(rep_t src,rep_t dst);
double applyHeuristics_ggp(heuristics_t h,rep_t rep,int side, int budget);
int estimateTreeSize_ggp(int treeSize);
void printBoard_ggp(rep_t rep, int dummy);

// Heuristic routines
double h1_ggp(rep_t rep, int side, int horizion);

#endif //GGP_H_INCLUDED