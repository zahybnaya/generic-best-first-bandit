#include "domain.h"
#include "mancala.h"
#include "synth.h"
#include "chess.h"
#include "zop.h"
#include "c4.h"
#include "sailing.h"
#include "ggp.h"


DOM* init_domain(DOM_NAME name){
	DOM* retVal = (DOM*)malloc(sizeof(DOM));
	switch (name) {
		case GGP:
			retVal->dom_name = GGP;
			retVal->getNumOfChildren = getNumOfChildren_ggp;
			retVal->estimateTreeSize = estimateTreeSize_ggp;
			retVal->isValidChild = isValidChild_ggp;
			retVal->applyHeuristics = applyHeuristics_ggp;
			retVal->cloneRep = cloneRep_ggp;
			retVal->makeMove = makeMove_ggp;
			retVal->getGameStatus = getGameStatus_ggp;
			retVal->destructRep = destructRep_ggp;
			retVal->allocate = allocate_ggp;
			retVal->generateRandomStart = generateRandomStart_ggp;
			retVal->copy = copy_ggp;
			retVal->printBoard = printBoard_ggp;
			retVal->hFunctions.h1  = h1_ggp;
			break;
		case SAILING:
			retVal->dom_name = SAILING;
			retVal->getNumOfChildren = getNumOfChildren_sailing;
			retVal->estimateTreeSize = estimateTreeSize_sailing;
			retVal->isValidChild = isValidChild_sailing;
			retVal->applyHeuristics = applyHeuristics_sailing;
			retVal->cloneRep = cloneRep_sailing;
			retVal->makeMove = makeMove_sailing;
			retVal->getGameStatus = getGameStatus_sailing;
			retVal->destructRep = destructRep_sailing;
			retVal->allocate = allocate_sailing;
			retVal->generateRandomStart = generateRandomStart_sailing;
			retVal->copy = copy_sailing;
			retVal->printBoard = printBoard_sailing;
			retVal->hFunctions.h1  = h1_sailing;
			retVal->hFunctions.h2  = h2_sailing;
			retVal->hFunctions.h3  = h3_sailing;
			retVal->hFunctions.h4  = h4_sailing;
			retVal->hFunctions.h5  = h5_sailing;
			retVal->hFunctions.h6  = h6_sailing;
			break;
		case C4:
			retVal->dom_name=C4;
			retVal->getNumOfChildren = getNumOfChildren_connect4;
			retVal->estimateTreeSize  = estimateTreeSize_connect4;
			retVal->isValidChild = isValidChild_connect4;
			retVal->applyHeuristics = applyHeuristics_connect4;
			retVal->cloneRep = cloneRep_connect4;
			retVal->makeMove = makeMove_connect4;
			retVal->getGameStatus = getGameStatus_connect4;
			retVal->destructRep = destructRep_connect4;
			retVal->allocate = allocate_connect4;
			retVal->generateRandomStart = generateRandomStart_connect4;
			retVal->copy = copy_connect4;
			retVal->printBoard = printBoard_c4;
			retVal->hFunctions.h1  = h3_c4;
			retVal->hFunctions.h2  = h3_c4;
			retVal->hFunctions.h3  = h3_c4;
			retVal->hFunctions.h4  = h3_c4;
			retVal->hFunctions.h5  = h3_c4;
			retVal->hFunctions.h6  = h3_c4;
			break;
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
			retVal->printBoard = printBoard_mancala;
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
			//retVal->printBoard = printBoard_synth;
			retVal->hFunctions.h1  = h1_synth;
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
		case ZOP: 
			retVal->dom_name=ZOP ;
			retVal->getNumOfChildren = getNumOfChildren_zop;
			retVal->estimateTreeSize  = estimatedTreeSize_zop;
			retVal->isValidChild = isValidChild_zop;
			retVal->applyHeuristics = applyHeuristics_zop;
			retVal->cloneRep = cloneRep_zop;
			retVal->makeMove = makeMove_zop;
			retVal->getGameStatus = getGameStatus_zop;
			retVal->destructRep = destructRep_zop;
			retVal->allocate = allocate_zop;
			retVal->generateRandomStart = generateRandomStart_zop;
			retVal->copy = copy_zop;
			retVal->hFunctions.h1  = h1_zop;
			retVal->hFunctions.h2  = h1_zop;
			retVal->hFunctions.h3  = h1_zop;
			retVal->hFunctions.h4  = h1_zop;
			retVal->hFunctions.h5  = h1_zop;
			retVal->hFunctions.h6  = h1_zop;
			break;
	}
	return retVal;
}
