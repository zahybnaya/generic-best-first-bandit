#ifndef MANCALA_H_INCLUDED
#define MANCALA_H_INCLUDED

#include "domain.h"
#define ESTIMATED_MANCALA_BRANCHING_FACTOR 3.4

/*
 * This macro generates a real "continues" array called __board
 * */
#define TO_REGULAR_ARR(board) \
    int __board[2][NUM_PITS+1];\
    int i_TRA,pit_TRA;\
    for(i_TRA=0;i_TRA<2;i_TRA++)for(pit_TRA=0;pit_TRA<NUM_PITS+1;pit_TRA++)__board[i_TRA][pit_TRA]=board[i_TRA][pit_TRA];\

#define COPY_ARR(src,dst){\
int i_CA,pit_CA; \
   for(i_CA=0;i_CA<2;i_CA++)for(pit_CA=0;pit_CA<NUM_PITS+1;pit_CA++)dst[i_CA][pit_CA]=src[i_CA][pit_CA];\
}


int getNumOfChildren_mancala(rep_t rep, int side);
int estimateTreeSize_mancala(int treeSize);
int isValidChild_mancala(rep_t rep, int side, int move);
int getGameStatus_mancala(rep_t rep);
void makeMove_mancala(rep_t rep,int * side, int move);
rep_t cloneRep_mancala(rep_t orig);
double applyHeuristics_mancala (heuristics_t h,rep_t rep,int side, int budget);
void generateRandomStart_mancala(rep_t rep,int*);
rep_t allocate_mancala();
void destructRep_mancala(rep_t rep);
void copy_mancala(rep_t src,rep_t dst);

// Heuristic routines (heuristic.c)
double h1(rep_t rep, int side, int dummy);
double h2(rep_t rep, int side, int dummy);
double h3(rep_t rep, int side, int numPlayouts);
double h4(rep_t rep, int side, int dummy);
double h5(rep_t rep, int side, int dummy);
double h6(rep_t rep, int side, int numPlayouts);

#endif // MANCALA_H_INCLUDED
