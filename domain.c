#include "domain.h"
#include "mancala.h"
#include "synth.h"
#include "chess.h"


DOM* init_domain(DOM_NAME name){
    DOM* retVal = (DOM*)malloc(sizeof(DOM));
    switch (name){
        case MANCALA:
            retVal->dom_name=MANCALA;
            retVal->getNumOfChildren = getNumOfChildren_mancala;
            retVal->estimateTreeSize  = estimateTreeSize_mancala;
            retVal->isValidChild = isValidChild_mancala;
            retVal->applyHeuristics = applyHeuristics_mancala;
            retVal->cloneRep = cloneRep_mancala;
            retVal->makeMove = makeMove_mancala;
            retVal->getGameStatus = getGameStatus_mancala;
            retVal->destructRep = destructRep_mancala;
            retVal->allocate = allocate_mancala;
            retVal->generateRandomStart = generateRandomStart_mancala;
            retVal->copy = copy_mancala;
	    retVal->hFunctions.h1  = h1; 
	    retVal->hFunctions.h2  = h2; 
	    retVal->hFunctions.h3  = h3; 
	    retVal->hFunctions.h4  = h4; 
	    retVal->hFunctions.h5  = h5; 
	    retVal->hFunctions.h6  = h6; 
            break;
        case SYNTH:
            retVal->dom_name=SYNTH;
            retVal->getNumOfChildren = getNumOfChildren_synth;
            retVal->estimateTreeSize  = estimateTreeSize_synth;
            retVal->isValidChild = isValidChild_synth;
            retVal->applyHeuristics = applyHeuristics_synth;
            retVal->cloneRep = cloneRep_synth;
            retVal->makeMove = makeMove_synth;
            retVal->getGameStatus = getGameStatus_synth;
            retVal->destructRep = destructRep_synth;
            retVal->allocate = allocate_synth;
            retVal->generateRandomStart = generateRandomStart_synth;
            retVal->copy = copy_synth;	  
	    retVal->hFunctions.h1  = h1; 
	    retVal->hFunctions.h2  = h2; 
	    retVal->hFunctions.h3  = h3; 
	    retVal->hFunctions.h4  = h4; 
	    retVal->hFunctions.h5  = h5; 
	    retVal->hFunctions.h6  = h6; 
        break;
        case CHESS:
//            retVal->dom_name=CHESS;
//            retVal->getNumOfChildren = getNumOfChildren_chess;
//            retVal->estimateTreeSize  = estimateTreeSize_chess;
//            retVal->isValidChild = isValidChild_chess;
//            retVal->applyHeuristics = applyHeuristics_chess;
//            retVal->cloneRep = cloneRep_chess;
//            retVal->makeMove = makeMove_chess;
//            retVal->getGameStatus = getGameStatus_chess;
//            retVal->destructRep = destructRep_chess;
//            retVal->allocate = allocate_chess;
//            retVal->generateRandomStart = generateRandomStart_chess;
//            retVal->copy = copy_chess;
        break;
    }
    return retVal;
}
