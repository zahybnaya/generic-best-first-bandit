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
#define BFB 512
#define BRUE 1024

//Type systems for BFB
#define MM_ORACLE 1
#define STS 2
#define VTS 3
#define CITS 4

//Type selection policies for BFB
#define MAB 1
#define KEEP_VMAB 2
#define DELETE_VMAB 3

// UCT back-up operator choices
#define AVERAGE 0
// Note that MINMAX is already defined above as 1, so we'll just use that
#define CI 2
#define WEIGHTED_MM 3
#define VARIANCE 4
#define VARIANCE_ALL 5
#define SIZE 6
#define CI_ALL 7
#define COULOM 8
#define WILCOXON 9

// Useful constants to index into arrays. For example, can use heuristic[max] to refer to the heuristic
// the max player is using.
#define max 0
#define min 1
#define store 0

// Constants relevant to reading board files
#define READ 0
#define WRITE 1
#define CLOSE 2

// Other constants
#define NULL_MOVE -1 // constant to indicate invalid moves; used for initialization purposes
#define EPSILON 0.1 /* if the MM-k player is noisy, then with probability given by EPSILON, the player picks the second-best move,
			provided that it is also within a score of 3 of the best move */

// Types
typedef struct timeval Timer;
typedef void* rep_t;
typedef unsigned int uid;
typedef double (*heuristics_t) (rep_t rep, int, int);
typedef unsigned short int BOOL;

// Macros
#define MISSING(x) { printf("Missing argument for option %s\n", x); return 0; }
#define OPTION(x) (strcmp(argv[i], x) == 0)
#define CHECK(p, mask, option) if (!(player[p] & mask)) { printf("Check use of option %s\n", option); return 0; }
#define MIN(x,y)(((x)<(y))?(x):(y))
#define MAX(x,y)(((x)<(y))?(y):(x))

// Global variables
extern int verbose; // sets whether output is verbose (defined in file with main() routine)

// Utility routines (util.c)
double getElapsed(Timer start);
Timer startTiming(void);
unsigned int devrand(void);
void swapDbls(double* val1, double* val2);
void swapPtrs(void** val1, void** val2);
void swapInts(int* val1, int* val2);

// UCT routines (uct.c)
int makeUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp, int ci_threshold, double *moveVals);
void genUCTTree(rep_t rep, int side, int numIterations, double C, heuristics_t heuristic, int budget);
void printUctStats();
int makeMinmaxOnUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int ci_threshold, double *moveVals);

//MMUCT (mmuct.c)
int makeMMUCTMove(rep_t rep, int *side, int numIterations, double C, heuristics_t h, int budget,
		int* bestMoves, int* numBestMoves, int backupOp,int mmTreeSize,int nodeLimit,uid* traps,int howManyTraps);
void printMmUctStats();
void resetTrapCounter();


// Minimax routines (minmax.c)
int makeMinmaxMove(rep_t rep, int* side, int depth, heuristics_t heuristic, int budget, int randomTieBreaks, int noisyMM, int* bestMoves, int* numBestMoves, double* termPercentage, double *moveVals) ;
int getAlphaBetaTreeSize(rep_t rep, int, int,  heuristics_t heuristic, int);
void genAlphaBetaTree(rep_t rep, int, int,  heuristics_t heuristic, int);
void setNoiseParams(int, double);

//BFB (bfb.c)
int makeBFBMove(rep_t rep, int *side, int tsId, int numIterations, double C, double CT, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp, int threshold, int policy);
void printBfbStats();

//BRUE (brue.c)
int makeBrueMove(rep_t rep, int *side, int numIterations, heuristics_t  heuristic, int* bestMoves, int* numBestMoves, int budget);

#endif //__COMMON__H__
