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
	    	int goldStandard);
	
static int maxVal(const double vals[], int length);
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int backOp, int ci_threshold);
void buildCITree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budge, int ci_threshold, int goldStandard);

void printMsg() {
	puts(" Evaluates individual states." );
	puts(" -g number of games ");
	puts(" -t ci minimal threshold ");
	puts(" -i number of iterations ") ;
	puts(" -s seed ") ;
	puts(" -h heuristic ") ;
	puts(" -gs gold standard level ") ;
}

int main(int argc, char* argv[]) {

	int s, side = max, numBestMoves, arg = 0;
	rep_t testState;

	/* defaults */
	_DOM = init_domain(MANCALA); //currently mancala
	int numGames = 10;
	int bestMoves[_DOM->getNumOfChildren()];
	int numIterations = 5000;
	heuristics_t heuristic =  _DOM->hFunctions.h3;
	int budget[2] = {1,1};
	int noisyMM = false;
	double C[2] = {2.5,2.5};
	int gs_level = 12,hfunc=3;
	double termPercentage;
	int ci_threshold = 30;
	int randomTieBreaks = false; // determine whether the MM player will break ties randomly
	unsigned int seed = 0;
 	/** parameters */
	for (arg=0;arg<argc;arg++){
		if (strcmp(argv[arg],"?")==0){
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
		} else if (strcmp(argv[arg],"-h")==0){
			hfunc=atoi(argv[++arg]);
			switch(hfunc){
			       	case 1:
					heuristic =  _DOM->hFunctions.h1;
				       	break;
			       	case 2:
					heuristic =  _DOM->hFunctions.h2;
					break;
				case 3:
					heuristic =  _DOM->hFunctions.h3;
					break;
				case 4:
					heuristic =  _DOM->hFunctions.h4;
					break;
				case 5:
					heuristic =  _DOM->hFunctions.h5;
					break;
				case 6:
					heuristic =  _DOM->hFunctions.h6;
					break;
			}
		}

	}
	
	printf("Seed, Instance, GS_level,  CI_Threshold, iterations, H , GS_Move, GS_Move_Val, CIB_Parental_Percantage , CIB_Average_depth, CIB_Min_Depth, CIB_Tree_Depth, CIB_Move, CIB_Move_Val, CIB_GS_Move_Val, UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, Parental_Percantage , Average_depth, Min_Depth, Tree_Depth, CI_Move, CI_Move_Val, CI_GS_Move_Val, MMF_Move, MMF_Move_Val, MMF_GS_Val\n");

	double *mmVals = calloc(_DOM->getNumOfChildren(), sizeof(double));
	
	for (s = 0; s < numGames; s++) {
		srandom(seed+s);
		testState = _DOM->allocate();
		_DOM->generateRandomStart(testState,&side);
		//Seed, Instance, GS_level,  CI_Threshold, iterations,h
		printf("%d, %d, %d, %d, %d, h%d, ", seed+s, s, gs_level, ci_threshold, numIterations, hfunc);
		//Make minmax move to depth 16
		srandom(seed);
		side = max;
		makeMinmaxMove(testState, &side, gs_level , heuristic, budget[side], randomTieBreaks, noisyMM, bestMoves, &numBestMoves, &termPercentage, mmVals);
		int goldStandard = maxVal(mmVals, _DOM->getNumOfChildren());
		//GS_Move, GS_Move_Val,
		printf("%d, %f, ", goldStandard , mmVals[goldStandard]);
		
		side = max;
		buildCITree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard);
		
		side = max;
		foldTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard);
		
		printf("\n");
		_DOM->destructRep(testState);
	}
	
	free(mmVals);
	return 0;
}

void buildCITree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int ci_threshold, int goldStandard) {
		int i;
		double cib_vals[_DOM->getNumOfChildren()];
		
		srandom(0);
		treeNode* tree = buildUCTTree(rep, side, numIterations, C, heuristic, budget, CI, ci_threshold);
		
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (tree->children[i]) { // if this child was explored
				cib_vals[i-1]= tree->children[i]->scoreSum / (double)tree->children[i]->n;
				if (*side == min)
					cib_vals[i-1]= -cib_vals[i-1];
			} else {
				assert(i>0);
				cib_vals[i-1] = -1 * INF;
			}
		}
		
		freeTree(tree);
		int bestMoveOfCIB = maxVal(cib_vals, _DOM->getNumOfChildren());
		
		//CIB_Move, CIB_Move_Val, CIB_GS_Move_Val, 
		printf("%d, %f, %f, ", bestMoveOfCIB, cib_vals[bestMoveOfCIB], cib_vals[goldStandard]); 
}

/**
 * Builds the UCT tree 
 * */
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget, int backOp, int ci_threshold) {
	int i;
	int parentCIWin=0,parentCITotal=0,minDepth=-INF,treeDepth=0;
	double avgDepth=0;
	
	treeNode* rootNode = createRootNode(side,rep);
	for (i = 0; i < numIterations; i++)
		uctRecurse(rootNode, C, heuristic, budget, backOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
	
	if (backOp == CI) {
	  //CIB_Parental_Percantage , CIB_Average_depth, CIB_Min_Depth, CIB_Tree_Depth
	  printf("%f , %f, %d, %d, " ,parentCIWin / (double)parentCITotal, avgDepth, minDepth, treeDepth);
	}
	
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
	    	int goldStandard) {
	int i,parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0;
	srandom(0);
	treeNode* tree = buildUCTTree(rep, side, numIterations, C, heuristic, budget, AVERAGE, INF);
	double uct_vals[_DOM->getNumOfChildren()];
	double uct_mm_vals[_DOM->getNumOfChildren()];
	double uct_ci_vals[_DOM->getNumOfChildren()];
	
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (tree->children[i]) { // if this child was explored
			uct_vals[i-1]= tree->children[i]->scoreSum / (double)tree->children[i]->n;
			uct_mm_vals[i-1]= minmaxUCT(tree->children[i]);
			vci *VCI = vciMinmaxUCT(tree->children[i], ci_threshold,&parentCIWin,&parentCITotal,&avgDepth,&minDepth,&treeDepth); // do a minmax backup this child
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

	//UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, 
	printf("%d, %f , %f, ",bestMoveOfUct, uct_vals[bestMoveOfUct], uct_vals[goldStandard]); 
	//Parental_Percantage , Average_depth, Min_Depth, Tree_Depth
	printf("%f , %f, %d, %d, " ,parentCIWin / (double)parentCITotal, avgDepth, minDepth, treeDepth);
	//CI_Move, CI_Move_Val, CI_GS_Move_Val, 
	printf("%d, %f , %f, ",bestMoveOfUct_ci, uct_ci_vals[bestMoveOfUct_ci], uct_ci_vals[goldStandard]); 
	//MMF_Move, MMF_Move_Val, MMF_GS_Val\n");
	printf("%d, %f , %f",bestMoveOfUct_mm, uct_mm_vals[bestMoveOfUct_mm], uct_mm_vals[goldStandard]); 
	parentCIWin = 0;
	parentCITotal = 0;
	freeTree(tree);
}


