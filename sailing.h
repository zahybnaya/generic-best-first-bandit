#ifndef SAILING_H_INCLUDED
#define SAILING_H_INCLUDED

#include "domain.h"

#define SAILING_BOARD_SIZE 10 //Board grid of size nXn
#define SAILING_DIRECTIONS 8 //Can sail to al 45 degree directions
#define SAILING_TACK_RANGE 3 //Tack direction. 0 - left tack, 1 - in tack, 2 - right tack. //currently in use only for value iteration
#define SAILING_WIND_CHANGE_PROB 0.3 //Probability of wind changing by 45 degrees in one direction (total prob of wind changing is 0.6)
#define SAILING_REP_SIZE 7 //The size of a sailing domain state representation
#define SAILING_DELAY 3 //the cost of a tack change
#define SAILING_HORIZION SAILING_BOARD_SIZE * SAILING_BOARD_SIZE

#define SAILING_STATE_TYPE 0 //Wheter this state is a chance node or not
#define BOAT_X 1 //The x coordination of the boat
#define BOAT_Y 2 //The y coordination of the boat
#define TACK 3   //The side of the boat which the wind was previously hitting
#define WIND 4   //The direction of the wind
#define GOAL_X 5 //The x coordination of the goal
#define GOAL_Y 6 //The y coordination of the goal

#define SAILING_STATE_DET 0    //A deterministic state
#define SAILING_STATE_CHANCE 1 //A stochastic state

int selectMoveStochastic_sailing(rep_t rep);
int isChanceNode_sailing(rep_t rep);
double actionCost_sailing(rep_t rep, int move);
int getNumOfChildren_sailing(rep_t rep, int side);
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
int *generateWeather(int wind);

// Heuristic routines
double h1_sailing(rep_t rep, int side, int horizion);
double h2_sailing(rep_t rep, int side, int horizion);
double h3_sailing(rep_t rep, int side, int horizion);
double h4_sailing(rep_t rep, int side, int horizion);
double h5_sailing(rep_t rep, int side, int horizion);
double h6_sailing(rep_t rep, int side, int horizion);

//value_iteration.c
double ****value_iteration();

#endif //SAILING_H_INCLUDED