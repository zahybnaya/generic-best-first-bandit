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

static int maxVal(const double vals[], int length);
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int backOp, int ci_threshold,double goldvalue);
void buildTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budge, int ci_threshold, int goldStandard, int backOp);

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
	
	//Header for tree building algorithms
	printf("goldvalue, reward, visit, average\n");
	double *mmVals = (double*)calloc(_DOM->getNumOfChildren(), sizeof(double));
	for (s = 0; s < numGames; s++) {
		srandom(seed+s);
		testState = _DOM->allocate();
		_DOM->generateRandomStart(testState,&side);
		//Make minmax move to depth 16
		side = max;
		makeMinmaxMove(testState, &side, gs_level , heuristic, budget[side], randomTieBreaks, noisyMM, bestMoves, &numBestMoves, &termPercentage, mmVals);
		int goldStandard = maxVal(mmVals, _DOM->getNumOfChildren());
		side = max;
		treeNode* tree = buildUCTTree(testState, &side, numIterations, C[side], heuristic, budget[side], AVERAGE, ci_threshold, mmVals[goldStandard]);
		printf("\n");
		_DOM->destructRep(testState);
		freeTree(tree);
	}
	free(mmVals);
	return 0;
}

/**
 * Builds the UCT tree 
 * */
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget, int backOp, int ci_threshold, double goldenValue) {
	int i;
	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0, r;
	
	treeNode* rootNode = createRootNode(side,rep);
	for (i = 0; i < numIterations; i++){
		r = uctRecurse(rootNode, C, heuristic, budget, backOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
		printf("%.2f, %.2f, %d, %.5f\n", goldenValue, r, i, rootNode->realScoreSum/rootNode->n);
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

