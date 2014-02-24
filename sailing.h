#ifndef SAILING_H_INCLUDED
#define SAILING_H_INCLUDED

#include "domain.h"

#define SAILING_BOARD_SIZE 5
#define SAILING_DIRECTIONS 8
#define SAILING_WIND_CHANGE_PROB 0.3
#define SAILING_REWARD 1000
#define SAILING_INCOMPLETE (SAILING_REWARD + 1)
#define BOAT_X 0
#define BOAT_Y 1
#define WIND 2
#define GOAL_X 3
#define GOAL_Y 4

int getNumOfChildren_sailing();
int isValidChild_sailing(rep_t rep, int side, int move);
int getGameStatus_sailing(rep_t rep);
void makeMove_sailing(rep_t rep,int * side, int move);
rep_t cloneRep_sailing(rep_t orig);
void generateRandomStart_sailing(rep_t rep,int*);
rep_t allocate_sailing();
void destructRep_sailing(rep_t rep);
void copy_sailing(rep_t src,rep_t dst);
double applyHeuristics_sailing(heuristics_t h,rep_t rep,int side, int budget);
int estimateTreeSize_sailing(int treeSize);
void printBoard_sailing(rep_t rep, int dummy);

// Heuristic routines
double h1_sailing(rep_t rep, int side, int horizion);
double h2_sailing(rep_t rep, int side, int horizion);
double h3_sailing(rep_t rep, int side, int horizion);
double h4_sailing(rep_t rep, int side, int horizion);
double h5_sailing(rep_t rep, int side, int horizion);
double h6_sailing(rep_t rep, int side, int horizion);

#endif //SAILING_H_INCLUDED