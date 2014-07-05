#include "sailing.h"
#include "common.h"

#define GAMMA 1

static double ****init_state_space_value_matrix() {
  int x, y, w;
  
  double ****mat = (double ****)calloc(SAILING_BOARD_SIZE, sizeof(double ***));
  for (x = 0; x < SAILING_BOARD_SIZE; x++) {
    mat[x] = (double ***)calloc(SAILING_BOARD_SIZE, sizeof(double **));
    
    for (y = 0; y < SAILING_BOARD_SIZE; y++) {
      mat[x][y] = (double **)calloc(SAILING_DIRECTIONS, sizeof(double *));
      
      for (w = 0; w < SAILING_DIRECTIONS; w++)
	mat[x][y][w] = (double *)calloc(SAILING_TACK_RANGE, sizeof(double));
    }
  }
  
  return mat;
}

static void switchMatrices(double *****mat1, double *****mat2) {
  double ****temp;
  temp = *mat1;
  *mat1 = *mat2;
  *mat2 = temp;
}

static void freeMatrix(double ****mat) {
  int x, y, w;
  
  for (x = 0; x < SAILING_BOARD_SIZE; x++) {
      for (y = 0; y < SAILING_BOARD_SIZE; y++) {
	for (w = 0; w < SAILING_DIRECTIONS; w++)
	  free(mat[x][y][w]);
	
	free(mat[x][y]);
      }
      free(mat[x]);
  }
  free(mat);
}

static void fillState(int *rep, int x, int y, int w, int t) {
  //in the game rep wind is 1-based and tack is -1, 0, 1.
  rep[SAILING_STATE_TYPE] = SAILING_STATE_DET;
  rep[BOAT_X] = x;
  rep[BOAT_Y] = y;
  rep[WIND] = w + 1;
  rep[TACK] = t - 1;
  rep[GOAL_X] = SAILING_BOARD_SIZE - 1;
  rep[GOAL_Y] = SAILING_BOARD_SIZE - 1;
}

static double computeQsa(double ****V, int *rep, int a) {
  double qsa = actionCost_sailing(rep, a + 1);
  
  makeMove_sailing(rep, 0, a + 1);
	      
  int newWind = rep[WIND] - 1; //shift wind back to zero based
  if (newWind == 0)
    newWind = SAILING_DIRECTIONS - 1;
  else
    newWind--;
	      
  qsa += GAMMA * SAILING_WIND_CHANGE_PROB * V[rep[BOAT_X]][rep[BOAT_Y]][newWind][rep[TACK] + 1]; //Wind moves left

  qsa += GAMMA * (1 - 2 * SAILING_WIND_CHANGE_PROB) * V[rep[BOAT_X]][rep[BOAT_Y]][rep[WIND] - 1][rep[TACK] + 1]; //Wind doesn't change

  newWind = rep[WIND] - 1; //shift wind back to zero based
  if (newWind == SAILING_DIRECTIONS - 1)
    newWind = 0;
  else
    newWind++;
	      
  qsa += GAMMA * SAILING_WIND_CHANGE_PROB * V[rep[BOAT_X]][rep[BOAT_Y]][newWind][rep[TACK] + 1]; //Wind moves right 
  
  return qsa;
}
/**
static void print_mat (double ****V1, double ****V2) {
  int x, y, w, t;
  for (x = 0; x < SAILING_BOARD_SIZE; x++) {
      for (y = 0; y < SAILING_BOARD_SIZE; y++) {
	for (w = 0; w < SAILING_DIRECTIONS; w++) {
	  for (t = 0; t < SAILING_TACK_RANGE; t++) {
	    printf("%d %d %d %d %f %f\n", x, y, w, t, V1[x][y][w][t], V2[x][y][w][t]);
	  }
	}
      }
  }
}
*/
double ****value_iteration() {
  double ****V1 = init_state_space_value_matrix();
  double ****V2 = init_state_space_value_matrix();
  
  double delta;
  int x, y, w, t, a;
  int *stateRep = (int *)calloc(SAILING_REP_SIZE, sizeof(int));
  do {
    delta = 0;
    
    for (x = 0; x < SAILING_BOARD_SIZE; x++)
      for (y = 0; y < SAILING_BOARD_SIZE; y++)
	for (w = 0; w < SAILING_DIRECTIONS; w++)
	  for (t = 0; t < SAILING_TACK_RANGE; t++) {
	    fillState(stateRep, x, y, w, t);
	    
	    if (getGameStatus_sailing(stateRep) != SAILING_INCOMPLETE)
	      continue;
	    
	    double maxQsa = -INF;
	    for (a = 0; a < SAILING_DIRECTIONS; a++) {
	      fillState(stateRep, x, y, w, t);
	      
	      if (isValidChild_sailing(stateRep, 0, a + 1) == false)
		continue;
	      
	      double qsa = computeQsa(V1, stateRep, a);
	      if (qsa > maxQsa)
		maxQsa = qsa;
	    }
	    
	    V2[x][y][w][t] = maxQsa;
	    double aDelta = abs(V1[x][y][w][t] - V2[x][y][w][t]);
	    if (aDelta > delta)
	      delta = aDelta;
	  }
        //printf("delta %f\n", delta);
	switchMatrices(&V1, &V2);
  } while (delta > EPSILON);
  
  
  //print_mat(V1, V2);
  //printf("\n\n");
  
  printf("yoyoyoyo %f\n", V2[0][0][0][1]);
  free(stateRep);
  freeMatrix(V1);
  
  return V2;
}