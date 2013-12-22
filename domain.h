#ifndef DOMAIN_H_INCLUDED
#define DOMAIN_H_INCLUDED
#include "common.h"


/***
 * This is a representation of a domain
 * rep_t is a void* defined in common.h
 * */


typedef int (*getNumOfChildren_func)();
typedef int (*estimatedTreeSize_func)(int treeSize);
typedef int (*isValidChild_func)(rep_t rep, int side,int move);
typedef int (*getGameStatus_func)(rep_t rep);
typedef void (*makeMove_func)(rep_t rep,int * side, int move);
typedef rep_t (*cloneRep_func)(rep_t orig);
typedef double (*applyHeuristics_func)(heuristics_t h,rep_t rep,int side, int budget);
typedef void (*destructRep_func) (rep_t rep);
typedef rep_t (*allocate_func)();
typedef void (*generateRandomStart_func)(rep_t state, int side);
typedef void (*copy_func) (rep_t src ,rep_t dest);
typedef enum {MANCALA =0 ,SYNTH =1 ,CHESS =2, ZOP=3} DOM_NAME;

/* Holds only the functions*/
typedef struct {
    DOM_NAME dom_name;
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
