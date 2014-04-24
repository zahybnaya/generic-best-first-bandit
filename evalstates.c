#include "common.h"
#include "domain.h"
#include "uct.c"
DOM* _DOM;
/***
 * Evaluate single states using different polices
 * */
int verbose = false;
int debuglog = false;
int logIteration=0;
static void foldTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic,
		int budget, int ci_threshold, 
	    	double gs_values[]);
	
static  int maxVal(const double vals[],int length);


void printMsg(){

	puts(" Evaluates individual states." );
	puts(" -g number of games ");
	puts(" -t ci minimal threshold ");
	puts(" -i number of iterations ") ;
	puts(" -s seed ") ;
	puts(" -gs gold standard level ") ;
}
int main(int argc, char* argv[]){

	int s, side = max,numBestMoves, arg=0;
	rep_t testState;

	/* defaults */
	_DOM = init_domain(MANCALA); //currently mancala
	int numGames = 10;
	int bestMoves[_DOM->getNumOfChildren()];
	int numIterations = 5000;
	heuristics_t heuristic[] =  {_DOM->hFunctions.h3, _DOM->hFunctions.h3}; // the heuristics used by the max and min playersa 
	int budget[2] = {1,1};
	int noisyMM = false;
	double C[2] = {2.5,2.5};
	int gs_level = 12;
	double termPercentage;
	int ci_threshold = 30;
	int randomTieBreaks = false; // determine whether the MM player will break ties randomly
	unsigned int seed = 0;
 	/** parameters */
	for (arg=0;arg<argc;arg++){
		if (strcmp(argv[arg],"-h")==0){
			printMsg();
			return -1;
		} else if (strcmp(argv[arg],"-g")==0){
			numGames = atoi(argv[++arg]);
		}else if (strcmp(argv[arg],"-t")==0){
			ci_threshold = atoi(argv[++arg]);
		} else if (strcmp(argv[arg],"-i")==0){
			numIterations = atoi(argv[++arg]);
		} else if (strcmp(argv[arg],"-s")==0){
			seed = (unsigned int)atoi(argv[++arg]);
		} else if (strcmp(argv[arg],"-gs")==0){
			gs_level = atoi(argv[++arg]);
		}

	}
		
	double *mmVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	double *uctVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	double *ciFoldingVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	double *mmFoldingVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	
	printf("Seed, Instance, GS_level,  CI_Threshold, iterations, GS_Move, GS_Move_Val, UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, Parental_Percantage , Average_depth, CI_Move, CI_Move_Val, CI_GS_Move_Val, MMF_Move, MMF_Move_Val, MMF_GS_Val\n");

	for (s = 0; s < numGames; s++) {
		srandom(seed+s);
		testState = _DOM->allocate();
		_DOM->generateRandomStart(testState,&side);
		printf("%d, %d, %d, %d, %d, ",seed+s, s, gs_level,ci_threshold, numIterations);
		//Make minmax move to depth 16
		srandom(seed);
		side = max;
		int mmMove = makeMinmaxMove(testState, &side, gs_level , heuristic[side], budget[side], randomTieBreaks, noisyMM, bestMoves, &numBestMoves, &termPercentage, mmVals);
		int goldStandard = maxVal(mmVals,_DOM->getNumOfChildren());
		printf("%d, %f, ", goldStandard , mmVals[goldStandard]);
		side = max;
		foldTree(testState, &side, numIterations, C[side], heuristic[side], budget[side], ci_threshold, mmVals);
		//srandom(seed);
		//side = max;
		////Make classic uct move (avg backprop)
		//int uctMove = makeUCTMove(testState, &side, numIterations, C[side], heuristic[side], budget[side], bestMoves, &numBestMoves, AVERAGE, INF, uctVals);
		//printf("%d, %f, %f ", uctMove, uctVals[uctMove], uctVals[mmMove]);
		////Make confidence folding uct move (avg backprop)
		//srandom(seed);
		//side = max;
		//int ciFoldingMove = makeMinmaxOnUCTMove(testState,&side,numIterations,C[side], heuristic[side], budget[side], bestMoves, &numBestMoves, ci_threshold, ciFoldingVals);
		//printf("%d, %f,%f, ", ciFoldingMove, ciFoldingVals[ciFoldingMove], ciFoldingVals[mmMove]);
		////Make minmax folding uct move (avg backprop)
		//srandom(seed);
		//side = max;
		//int mmFoldingMove = makeMinmaxOnUCTMove(testState, &side, numIterations, C[side], heuristic[side], budget[side], bestMoves, &numBestMoves, -1, mmFoldingVals);
		//printf("%d, %f, %f", mmFoldingMove, mmFoldingVals[mmFoldingMove], mmFoldingVals[mmMove]);
		printf("\n");
		_DOM->destructRep(testState);
	}
	free(mmVals);
	free(uctVals);
	free(ciFoldingVals);
	free(mmFoldingVals);
	return 0;
}

/**
 * Builds the UCT tree 
 * */
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget){
	int i;
	treeNode* rootNode = createRootNode(side,rep);
	for (i = 0; i < numIterations; i++)
		uctRecurse(rootNode, C, heuristic, budget, AVERAGE, true, INF);
	return rootNode;
}


int maxVal(const double vals[],int length){
	double cmax = -1*INF;
	int cmaxind=NULL_MOVE;
	int i;
	for (i=0;i<length;i++){
		if(vals[i]>cmax){
			cmaxind = i;
			cmax = vals[i];
		}
	}
	return cmaxind;
}

/* Runs specified number of iteration of UCT. Then, runs minimax on the resulting UCT tree and returns the best move */
void foldTree(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget,
		int ci_threshold, 
	    	double gs_values[]) {
	int i,parentCIWin=0,parentCITotal=0;
	double avgDepth=0;
	srandom(0);
	treeNode* tree = buildUCTTree(rep,side,numIterations,C,heuristic,budget);
	double uct_vals[_DOM->getNumOfChildren()];
	double uct_mm_vals[_DOM->getNumOfChildren()];
	double uct_ci_vals[_DOM->getNumOfChildren()];
	
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (tree->children[i]) { // if this child was explored
			uct_vals[i-1]= tree->children[i]->scoreSum/tree->children[i]->n;
			//printf("FOLD:%d:%f/%d\n",i,tree->children[i]->scoreSum,tree->children[i]->n);
			uct_mm_vals[i-1]= minmaxUCT(tree->children[i]);
			vci *VCI = vciMinmaxUCT(tree->children[i], ci_threshold,&parentCIWin,&parentCITotal,&avgDepth); // do a minmax backup of the subtree rooted at this child
			uct_ci_vals[i-1] = VCI->score;
			free(VCI);
			// negamax
			if (*side == min){
				uct_vals[i-1]= -uct_vals[i-1];
				uct_mm_vals[i-1]= -uct_mm_vals[i-1];
				uct_ci_vals[i-1]= -uct_ci_vals[i-1];
			}
		}else {
			assert(i>0);
			uct_vals[i-1]=uct_mm_vals[i-1]=uct_ci_vals[i-1] = -1*INF;
		}
	}

	int bestMoveOfUct = maxVal(uct_vals,_DOM->getNumOfChildren());
	int bestMoveOfUct_mm = maxVal(uct_mm_vals,_DOM->getNumOfChildren());
	int bestMoveOfUct_ci = maxVal(uct_ci_vals,_DOM->getNumOfChildren());
	int goldStandard = maxVal(gs_values,_DOM->getNumOfChildren());
	/* printouts :UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, Parental_Percantage , CI_Move, CI_Move_Val, CI_GS_Move_Val, MMF_Move, MMF_Move_Val, MMF_GS_Val\n");*/
	printf("%d, %f , %f, ",bestMoveOfUct, uct_vals[bestMoveOfUct], uct_vals[goldStandard]); 
	printf("%f ," ,parentCIWin / (double)parentCITotal);
	printf("%f ," ,avgDepth);
	printf("%d, %f , %f, ",bestMoveOfUct_ci, uct_ci_vals[bestMoveOfUct_ci], uct_ci_vals[goldStandard]); 
	printf("%d, %f , %f",bestMoveOfUct_mm, uct_mm_vals[bestMoveOfUct_mm], uct_mm_vals[goldStandard]); 
	parentCIWin = 0;
	parentCITotal = 0;
	// Clean up when done
	freeTree(tree);
	//int bestMoves[_DOM->getNumOfChildren()],numBestMoves;
	//double *uctVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	//*side = orignalSide;
	//srandom(0);
	//int uctMove = makeUCTMove(rep, side, numIterations, C,heuristic, budget, bestMoves, &numBestMoves, AVERAGE, INF, uctVals);
	////printf("%d %d\n",uctMove,bestMoveOfUct);
	////assert(uctMove == bestMoveOfUct);
	//for (i=0;i<_DOM->getNumOfChildren();i++){
	//	printf("%d:UCT:%f FOLD_TREE:%f\n",i,uctVals[i],uct_vals[i]);
	//	assert(abs(uctVals[i]-uct_vals[i])<0.001 || uct_vals[i]== -INF);
	//}
	//free(uctVals);
}


