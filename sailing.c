#include "sailing.h"

void printBoard_sailing(rep_t rep, int dummy) {
  int *game = rep;
  
  printf("Boat loc: (%d, %d)\n", game[BOAT_X], game[BOAT_Y]);
  printf("Wind dir: %d\n", game[WIND]);
  printf("Goal loc: (%d, %d)\n", game[GOAL_X], game[GOAL_Y]);
}

int getNumOfChildren_sailing() {
    return SAILING_DIRECTIONS + 1;
}

int isValidChild_sailing(rep_t rep, int side, int move) {
    int *game = rep;
    int x = game[BOAT_X];
    int y = game[BOAT_Y];
    int wind = game[WIND];
    
    //If this is a chance node then the only legal children are in the possible directions of the new wind (45 degrees left or right)
    if (game[SAILING_STATE_TYPE] == SAILING_STATE_CHANCE) {
      if ((wind == SAILING_DIRECTIONS && move == 1) || (wind < SAILING_DIRECTIONS && move == wind + 1))
	return true;
      
      if (wind == move)
	return true;
      
      if ((wind == 1 && move == SAILING_DIRECTIONS) || (wind > 1 && move == wind - 1))
	return true;

      return false;
    }
    
    //can't go against the wind
    if (abs(move - wind) == 4)
      return false;
    
    //check left x boundery
    if ((move == 1 || move == 2 || move == 3) && x == SAILING_BOARD_SIZE - 1)
      return false;
    
    //check right x boundery
    if ((move == 5 || move == 6 || move == 7) && x == 0)
      return false;
    
    //check upper y boundery
    if ((move == 0 || move == 1 || move == 7) && y == SAILING_BOARD_SIZE - 1)
      return false;
    
    //check lower y boundery
    if ((move == 4 || move == 3 || move == 5) && y == 0)
      return false;

    return true;
}

int getGameStatus_sailing(rep_t rep) {
  int *game = rep;
  
  if (game[BOAT_X] == game[GOAL_X] && game[BOAT_Y] == game[GOAL_Y])
    return SAILING_REWARD;
  
  return INCOMPLETE;
}

//if move is -1, and it is a chance node, than make a stochastic random move, else make the given move.
void makeMove_sailing(rep_t rep, int *side, int move) {
  int *game = rep;
  
  if (game[SAILING_STATE_TYPE] == SAILING_STATE_CHANCE) {
    if (move > 0) {
      game[WIND] = move;
      game[SAILING_STATE_TYPE] = SAILING_STATE_DET;
      return;
    }
    
    int windChange = random() % 100;
    
    if (0 <= windChange && windChange < SAILING_WIND_CHANGE_PROB * 100) {
      game[WIND]--;
      if (game[WIND] < 1)
	game[WIND] = SAILING_DIRECTIONS;
    } else if (SAILING_WIND_CHANGE_PROB * 100 <= windChange && windChange < SAILING_WIND_CHANGE_PROB * 100 * 2) {
      game[WIND]++;
      if (game[WIND] > SAILING_DIRECTIONS)
	game[WIND] = 1;
    }
    
    game[SAILING_STATE_TYPE] = SAILING_STATE_DET;
  } else {
    //x+1
    if (move == 1 || move == 2 || move == 3)
      game[BOAT_X]++;
    
    //x-1
    if (move == 5 || move == 6 || move == 7)
      game[BOAT_X]--;
    
    //y+1
    if (move == 0 || move == 1 || move == 7)
      game[BOAT_Y]++;
    
    //y-1
    if (move == 4 || move == 3 || move == 5)
      game[BOAT_Y]--;
    
    game[SAILING_STATE_TYPE] = SAILING_STATE_CHANCE;
  }
}

rep_t cloneRep_sailing(rep_t orig) {
   int i;
   int *game = orig;
   int *new_game = calloc(SAILING_REP_SIZE, sizeof(int));
   for (i = 0; i < SAILING_REP_SIZE; i++)
     new_game[i] = game[i];
   
   return new_game;
}

void generateRandomStart_sailing(rep_t rep, int *side) {
  int *game = rep;
  
  game[SAILING_STATE_TYPE] = SAILING_STATE_DET;
  
  game[BOAT_X] = random() % SAILING_BOARD_SIZE;
  game[BOAT_Y] = random() % SAILING_BOARD_SIZE;
  
  game[WIND] = random() % SAILING_DIRECTIONS + 1;
  
  game[GOAL_X] = random() % SAILING_BOARD_SIZE;
  game[GOAL_Y] = random() % SAILING_BOARD_SIZE;
}

rep_t allocate_sailing() {
  return calloc(SAILING_REP_SIZE, sizeof(int));
}

void destructRep_sailing(rep_t rep) {
  free(rep);
}

void copy_sailing(rep_t src, rep_t dst) {
  int i;
  for (i = 0; i < SAILING_REP_SIZE; i++)
     ((int *)dst)[i] = ((int *)src)[i];
}

double applyHeuristics_sailing(heuristics_t h, rep_t rep, int side, int budget) {

   rep_t clone = cloneRep_sailing(rep);
   double ret = h(clone, side, budget);
   destructRep_sailing(clone);
   
   return ret;
}

double h1_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

double h2_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

double h3_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

double h4_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

double h5_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

double h6_sailing(rep_t rep, int side, int horizion) {
  return 0;
}

int estimateTreeSize_sailing(int treeSize) {
  return -1;
}