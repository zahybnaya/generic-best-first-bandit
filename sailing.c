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
    
    //can't go against the wind
    if ((move + 4) % SAILING_DIRECTIONS == wind)
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
    return MAX_WINS;
  
  return INCOMPLETE;
}

void makeMove_sailing(rep_t rep, int *side, int move) {
  int *game = rep;
  
  //x+1
  if ((move == 1 || move == 2 || move == 3) && game[BOAT_X] < SAILING_BOARD_SIZE - 1)
    game[BOAT_X]++;
    
  //x-1
  if ((move == 5 || move == 6 || move == 7) && game[BOAT_X] > 0)
    game[BOAT_X]--;
    
  //y+1
  if ((move == 0 || move == 1 || move == 7) && game[BOAT_Y] < SAILING_BOARD_SIZE - 1)
    game[BOAT_Y]++;
    
  //y-1
  if ((move == 4 || move == 3 || move == 5) && game[BOAT_Y] > 0)
    game[BOAT_Y]--;
}

rep_t cloneRep_sailing(rep_t orig) {
   int i;
   int *game = orig;
   int *new_game = calloc(5, sizeof(int));
   for (i = 0; i < 5; i++)
     new_game[i] = game [i];
   
   return new_game;
}

void generateRandomStart_sailing(rep_t rep, int *side) {
  int *game = rep;
  
  game[BOAT_X] = random() % SAILING_BOARD_SIZE;
  game[BOAT_Y] = random() % SAILING_BOARD_SIZE;
  
  game[WIND] = random() % SAILING_DIRECTIONS;
  
  game[GOAL_X] = random() % SAILING_BOARD_SIZE;
  game[GOAL_Y] = random() % SAILING_BOARD_SIZE;
}

rep_t allocate_sailing() {
  return calloc(5, sizeof(int));
}

void destructRep_sailing(rep_t rep) {
  free(rep);
}

void copy_sailing(rep_t src, rep_t dst) {
  int i;
  for (i = 0; i < 5; i++)
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