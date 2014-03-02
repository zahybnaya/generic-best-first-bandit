#ifndef SAILING_H_INCLUDED
#define SAILING_H_INCLUDED

#include "domain.h"

#define SAILING_BOARD_SIZE 2 //Board grid of size nXn
#define SAILING_DIRECTIONS 8 //Can sail to al 45 degree directions
#define SAILING_WIND_CHANGE_PROB 1 //Probability of wind changing by 45 degrees
#define SAILING_REWARD (SAILING_BOARD_SIZE * 2) //Reward at goal
#define SAILING_INCOMPLETE (SAILING_REWARD + 1) //A flag to indicate wheter the game ended
#define SAILING_REP_SIZE 7 //The size of a sailing domain state representation
#define SAILING_DELAY 3 //the cost of a tack change

#define SAILING_STATE_TYPE 0 //Wheter this state is a chance node or not
#define BOAT_X 1 //The x coordination of the boat
#define BOAT_Y 2 //The y coordination of the boat
#define TACK 3   //The side of the boat which the wind was previously hitting
#define WIND 4   //The direction of the wind
#define GOAL_X 5 //The x coordination of the goal
#define GOAL_Y 6 //The y coordination of the goal

#define SAILING_STATE_DET 0    //A deterministic state
#define SAILING_STATE_CHANCE 1 //A stochastic state

int isChanceNode_sailing(rep_t rep);
double actionCost_sailing(rep_t rep, int move);
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