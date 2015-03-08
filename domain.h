#ifndef DOMAIN_H_INCLUDED
#define DOMAIN_H_INCLUDED
#include "common.h"
#include "mancala.h"
#include "synth.h"
#include "chess.h"
#include "zop.h"
#include "c4.h"
#include "sailing.h"

/***
 * This is a representation of a domain
 * rep_t is a void* defined in common.h
 * */


typedef int (*getNumOfChildren_func)(rep_t rep, int side);
typedef int (*estimatedTreeSize_func)(int treeSize);
typedef int (*isValidChild_func)(rep_t rep, int side,int move);
typedef int (*getGameStatus_func)(rep_t rep);
typedef void (*makeMove_func)(rep_t rep,int * side, int move);
typedef rep_t (*cloneRep_func)(rep_t orig);
typedef double (*applyHeuristics_func)(heuristics_t h,rep_t rep,int side, int budget);
typedef void (*destructRep_func) (rep_t rep);
typedef rep_t (*allocate_func)();
typedef void (*generateRandomStart_func)(rep_t state, int *side);
typedef void (*copy_func) (rep_t src ,rep_t dest);
typedef void (*printBoard_func)(rep_t rep, int extra);
typedef enum {MANCALA = 0 ,SYNTH = 1 ,CHESS = 2, ZOP = 3, C4 = 4, SAILING = 5, GGP = 6} DOM_NAME;

/* Holds only the functions*/
typedef struct {
    DOM_NAME dom_name;
    int max_wins;
    int min_wins;
    int draw;
    int incomplete;
    getNumOfChildren_func getNumOfChildren;
    estimatedTreeSize_func estimateTreeSize;
    isValidChild_func isValidChild;
    getGameStatus_func getGameStatus;
    makeMove_func makeMove;
    cloneRep_func cloneRep;
    applyHeuristics_func applyHeuristics;
    destructRep_func destructRep;
    allocate_func allocate;
    generateRandomStart_func generateRandomStart;
    copy_func copy;
    printBoard_func printBoard;
    struct {heuristics_t h1;
	    heuristics_t h2;
	    heuristics_t h3;
	    heuristics_t h4;
	    heuristics_t h5;
	    heuristics_t h6;
    } hFunctions;
} DOM;

DOM* init_domain(DOM_NAME dom_name);

#endif // DOMAIN_H_INCLUDED
