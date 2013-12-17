#ifndef __COMMON__H__
#define __COMMON__H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

// General constants
#define false 0
#define true 1
#define INF 999999999

// Bit masks for algorithm choices -- used in bounds-checking of command-line parameters
#define MINMAX 1
#define UCT 2
#define RANDOM 4
#define MINMAX_ON_UCT 8
#define COARSE_PLAYOUTS 16
#define FINE_PLAYOUTS 32
#define COARSE_HEURISTIC 64
#define FINE_HEURISTIC 128
#define MMUCT 256

// UCT back-up operator choices
#define AVERAGE 0
// Note that MINMAX is already defined above as 1, so we'll just use that

// Game parameters
#define NUM_PITS 6 // affects branching factor
#define SHELLS_PER_PIT 4 // affects depth of tree

// Useful constants to index into arrays. For example, can use heuristic[max] to refer to the heuristic
// the max player is using.
#define max 0
#define min 1
#define store 0

// Game status constants
#define MAX_WINS (NUM_PITS * SHELLS_PER_PIT * 2) // value of a win for max
#define MIN_WINS -MAX_WINS // value of a win for min
#define DRAW 0 // value of a draw
#define INCOMPLETE -1 // value of an incomplete game (to see whether a game has ended, compare return value of getGameStatus() to this)

// Constants relevant to reading board files
#define READ 0
#define WRITE 1
#define CLOSE 2

// Other constants
#define NULL_MOVE -1 // constant to indicate invalid moves; used for initialization purposes
#define EPSILON 0.35 /* if the MM-k player is noisy, then with probability given by EPSILON, the player picks the second-best move,
			provided that it is also within a score of 3 of the best move */

// Types
typedef struct timeval Timer;
typedef void* rep_t;
typedef unsigned int uid;
typedef double (*heuristics_t) (rep_t rep, int, int);

// Macros
#define MISSING(x) { printf("Missing argument for option %s\n", x); return 0; }
#define OPTION(x) (strcmp(argv[i], x) == 0)
#define CHECK(p, mask, option) if (!(player[p] & mask)) { printf("Check use of option %s\n", option); return 0; }
#define MIN(x,y)(((x)<(y))?(x):(y))
#define MAX(x,y)(((x)<(y))?(y):(x))


// Global variables
extern int verbose; // sets whether output is verbose (defined in file with main() routine)

/* Function prototypes */
// Board-related routines (board.c)
void initBoard(int board[2][NUM_PITS+1], int* side);
void genRandomBoard(int board[2][NUM_PITS+1], int* side, int depth);
void cloneBoard(int srcBoard[2][NUM_PITS+1] , int dstBoard[2][NUM_PITS+1]);
int readBoard(char* fileName, int board[2][NUM_PITS+1], int* side, short mode);
void writeBoard(char* fileName, int board[2][NUM_PITS+1], int side, short mode);
int isEqual(int board1[2][NUM_PITS+1], int board2[2][NUM_PITS+1]);
void printBoard(int board[2][NUM_PITS+1], int side);

// Game-rules related routines (move.c)
int getGameStatus(int board[2][NUM_PITS+1]);
void makeMove(int board[2][NUM_PITS+1] , int* side, int move);
int getNumLegalMoves(int board[2][NUM_PITS+1], int side);

// Utility routines (util.c)
double getElapsed(Timer start);
Timer startTiming(void);
unsigned int devrand(void);
void swapDbls(double* val1, double* val2);
void swapPtrs(void** val1, void** val2);
void swapInts(int* val1, int* val2);

// UCT routines (uct.c)
int makeUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp);
void genUCTTree(rep_t rep, int side, int numIterations, double C, heuristics_t heuristic, int budget);
void printUctStats();
int makeMinmaxOnUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves);


//MMUCT (mmuct.c)
int makeMMUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t h, int budget,
		int* bestMoves, int* numBestMoves, int backupOp,int mmTreeSize,int nodeLimit,uid* traps,int howManyTraps);
void printMmUctStats();
void resetTrapCounter();


// Minimax routines (minmax.c)
int makeMinmaxMove(rep_t rep, int* side, int depth, heuristics_t heuristic, int budget, int pruning, int randomTieBreaks, int noisyMM, int* bestMoves, int* numBestMoves, double* termPercentage) ;
int getAlphaBetaTreeSize(rep_t rep, int, int,  heuristics_t heuristic, int);
void genAlphaBetaTree(rep_t rep, int, int,  heuristics_t heuristic, int);

// Heuristic routines (heuristic.c)
double h1(int board[2][NUM_PITS+1], int side, int dummy);
double h2(int board[2][NUM_PITS+1], int side, int dummy);
double h3(int board[2][NUM_PITS+1], int side, int numPlayouts);
double h4(int board[2][NUM_PITS+1], int side, int dummy);
double h5(int board[2][NUM_PITS+1], int side, int dummy);
double h6(int board[2][NUM_PITS+1], int side, int numPlayouts);
void setNoiseParams(int, double);

// Random play routines (random.c)
int pickRandomMove(int board[2][NUM_PITS+1], int side);
int makeRandomMove(int board[2][NUM_PITS+1], int *side);

#endif //__COMMON__H__
