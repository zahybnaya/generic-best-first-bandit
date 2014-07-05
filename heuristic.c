#include "mancala.h"
#include "common.h"

/* Collection of leaf estimation routines for Mancala */

/***********************************************************************************************************
  Heuristics -- guidelines:

  1) Values returned need to be in the range (MANCALA_MIN_WINS, MANCALA_MAX_WINS) (note that the bounds are not inclusive)    

  2) Positive values should favor Max, negative values indicate position is favorable to Min.    

  3) While the first two parameters are used to pass in the game state, the third argument is optional and
  may be used to pass in any heuristic-specific info (for example, in h3, this is used to specify the
  number of playouts to perform)
 ************************************************************************************************************/

static short noisyHeuristic = false;
static int noiseMag = 0;
static double noiseProb = 0.0;

/* Sets the values of the global variables above */
void setNoiseParams(int delta, double p) {
	noisyHeuristic = true;
	noiseMag = delta;
	noiseProb = p;
}

/* Performs a single random playout and returns a value from the set {-1, 0, +1} */
static int playout(int board[2][NUM_PITS+1], int side) {
	int val;
	int dummyBoard[2][NUM_PITS+1];

	// To avoid clobbering original board, we operate on a copy
	cloneBoard(board, dummyBoard);

	// Play randomly to the end
	while ((val = getGameStatus(dummyBoard)) == MANCALA_INCOMPLETE)
		makeMove(dummyBoard, &side, pickRandomMove(dummyBoard, side));

	// Rescale outcome to value in the set {-1, 0, +1}
	val /= MANCALA_MAX_WINS;

	return val;
}


/* Performs a single random playout and returns the difference of the stores at the end
   of the game */
static int finerPlayout(int board[2][NUM_PITS+1], int side) {
	int val;
	int dummyBoard[2][NUM_PITS+1];

	// To avoid clobbering original board, we operate on a copy
	cloneBoard(board, dummyBoard);

	// Play randomly to the end
	while ((val = getGameStatus(dummyBoard)) == MANCALA_INCOMPLETE)
		makeMove(dummyBoard, &side, pickRandomMove(dummyBoard, side));

	// What is the difference of the stores at the end?
	val = dummyBoard[max][store] - board[min][store];

	return val;
}



/* Basic heuristic -- returns the difference of the stores. */
double h1(rep_t rep, int side, int dummy) {
	int ** board =(int**)rep;
	TO_REGULAR_ARR(board);

	double val = __board[max][store] - __board[min][store];
	double die;
	double noise;

	if (noisyHeuristic) { // if we are going to corrupt the heuristic
		die = (double)(random() % 10000) / (double)(10000 - 1);
		if (die <= noiseProb) { // coin-flip says this estimate should be corrupted
			noise = (random() % noiseMag) + 1; // randomly set magnitude of noise -- in the range {1, 2, ..., m}
		if (random() % 2) // randomly set sign of noise
			val += noise;
		else
			val -= noise;
		}
	}

	return val;
}


/* Alternate heuristic -- score each side using (# stones in own store - # stones on opponent's side)
   Return the difference of the scores */
double h2(rep_t rep, int side, int dummy) {
	int ** board =(int**)rep;
	TO_REGULAR_ARR(board);

	int i;
	double score;

	score = __board[max][store]-__board[min][store];
	for (i = 1; i < NUM_PITS+1; i++)
		score = score + __board[min][i] - __board[max][i];

	return score;
}


/* Playout-based heuristic -- perform specified number of random playouts and return 
   the average of the outcomes */
double h3(rep_t rep, int side, int numPlayouts) {
	int ** board =(int**)rep;
	TO_REGULAR_ARR(board);
	int playoutSum = 0;
	int i;

	for (i = 0; i < numPlayouts; i++)
		playoutSum += playout(__board, side);

	return (double)playoutSum/(double)numPlayouts;
}


/* Heuristic that returns a value at random from the set {-1, 0, 1}. This allows us to recognize true 
   terminal nodes, while providing a random evaluation of all non-terminal positions */
double h4(rep_t rep, int side, int dummy) {
	//int ** board =(int**)rep;
	//TO_REGULAR_ARR(board);
	double die;

	// Make sure that the values {-1, 0, 1} do not clash with MANCALA_MIN_WINS, MANCALA_MAX_WINS and MANCALA_DRAW
	assert ((MANCALA_MIN_WINS != -1) && (MANCALA_MIN_WINS != 0) && (MANCALA_MIN_WINS != 1));
	assert ((MANCALA_MAX_WINS != -1) && (MANCALA_MAX_WINS != 0) && (MANCALA_MAX_WINS != 1));
	assert ((MANCALA_DRAW != -1) && (MANCALA_DRAW != 0) && (MANCALA_DRAW != 1));

	die = (double)(random() % 10000) / (double)(10000 - 1);

	if (die <= 0.3333)
		return -1;
	else if (die <= 0.6667)
		return 0;

	return 1;
}


/* This is the coarsened version of h1 */
double h5(rep_t rep, int side, int dummy) {
	int ** board =(int**)rep;
	TO_REGULAR_ARR(board);
	double val = __board[max][store] - __board[min][store];

	// Take the difference of the stores and remap to {-1, 0, +1}
	if (val > 0)
		return +1;
	else if (val < 0)
		return -1;

	return 0;
}


/* Finer-grained playouts */
double h6(rep_t rep, int side, int numPlayouts) {
	int ** board =(int**)rep;
	TO_REGULAR_ARR(board);
	int playoutSum = 0;
	int i;

	for (i = 0; i < numPlayouts; i++)
		playoutSum += finerPlayout(__board, side);

	return (double)playoutSum/(double)numPlayouts;
}


