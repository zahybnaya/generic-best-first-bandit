#ifndef GGP_H_INCLUDED
#define GGP_H_INCLUDED

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "common.h"

EXTERNC int getNumOfChildren_ggp(rep_t rep, int side);
EXTERNC int isValidChild_ggp(rep_t rep, int side, int move);
EXTERNC int getGameStatus_ggp(rep_t rep);
EXTERNC void makeMove_ggp(rep_t rep,int * side, int move);
EXTERNC rep_t cloneRep_ggp(rep_t orig);
EXTERNC void generateRandomStart_ggp(rep_t rep,int*);
EXTERNC rep_t allocate_ggp();
EXTERNC void destructRep_ggp(rep_t rep);
EXTERNC void copy_ggp(rep_t src,rep_t dst);
EXTERNC double applyHeuristics_ggp(heuristics_t h,rep_t rep,int side, int budget);
EXTERNC int estimateTreeSize_ggp(int treeSize);
EXTERNC void printBoard_ggp(rep_t rep, int dummy);

// Heuristic routines
EXTERNC double h1_ggp(rep_t rep, int side, int horizion);

EXTERNC void compile_ggp();

#undef EXTERNC
#endif //GGP_H_INCLUDED