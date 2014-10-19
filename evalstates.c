#include "common.h"
#include "domain.h"
#include "uct.c"
#include "backup_operators.c"
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
void buildTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budge, int ci_threshold, int goldStandard, int backOp);

void printMsg() {
	puts(" Evaluates individual states." );
	puts(" -g number of games ");
	puts(" -t ci minimal threshold ");
	puts(" -i number of iterations ") ;
	puts(" -s seed ") ;
	puts(" -b segmentation for h4 ") ;
	puts(" -c exploration constant (9999 for random)") ;
	puts(" -h heuristic ") ;
	puts(" -gs gold standard level ") ;
}

int executeFoldTrees(int argc, char* argv[]) ;

int main(int argc, char* argv[]) {
	return executeFoldTrees(argc,argv);
}

int executeFoldTrees(int argc, char* argv[]) {

	int s, side = max, numBestMoves, arg = 0;
	rep_t testState;

	/* defaults */
	_DOM = init_domain(MANCALA); 
	int numGames = 10;
	rep_t dummy1 = 0;
	int dummy2= 0;
	int children_count = _DOM->getNumOfChildren(dummy1,dummy2);
	int bestMoves[children_count];
	int numIterations = 5000;
	heuristics_t heuristic =  _DOM->hFunctions.h3;
	int noisyMM = false;
	double C = 2.5;
	int gs_level = 12,hfunc=3;
	int segmentation = 1;
	int budget[2] = {1,1};
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
		}else if (strcmp(argv[arg],"-c")==0){
			C = atof(argv[++arg]);
		} else if (strcmp(argv[arg],"-i")==0){
			numIterations = atoi(argv[++arg]);
		} else if (strcmp(argv[arg],"-s")==0){
			seed = (unsigned int)atoi(argv[++arg]);
		} else if (strcmp(argv[arg],"-b")==0){
			segmentation = (unsigned int)atoi(argv[++arg]);
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
	
	budget[0] = segmentation;
	budget[1] = segmentation;
	//printf("Seed, Instance, GS_level,  CI_Threshold, iterations, H , GS_Move, GS_Move_Val, CIB_Parental_Percantage , CIB_Average_depth, CIB_Min_Depth, CIB_Tree_Depth, CIB_Move, CIB_Move_Val, CIB_GS_Move_Val, UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, Parental_Percantage , Average_depth, Min_Depth, Tree_Depth, CI_Move, CI_Move_Val, CI_GS_Move_Val, MMF_Move, MMF_Move_Val, MMF_GS_Val\n");
	
	//Header for tree building algorithms
	printf("Seed, Instance, GS_level,  Iterations, H ,Segmentation, GS_Move, GS_Move_Val,");
	printf(" UCT_value, CDP_value, MM_value\n");
	double *mmVals = (double*)calloc(children_count, sizeof(double));
	for (s = 0; s < numGames; s++) {
		srandom(seed+s);
		testState = _DOM->allocate();
		_DOM->generateRandomStart(testState,&side);
		//Seed, Instance, GS_level,  CI_Threshold, iterations, h, seg
		printf("%d, %d, %d, %d, h%d, %d", seed+s, s, gs_level,  numIterations, hfunc,segmentation );
		side = max;
		makeMinmaxMove(testState, &side, gs_level , heuristic, budget[side], randomTieBreaks, noisyMM, bestMoves, &numBestMoves, &termPercentage, mmVals);
		int goldStandard = maxVal(mmVals,children_count);
		//GS_Move, GS_Move_Val,
		printf(" , %d, %f, ", goldStandard , mmVals[goldStandard]);
		side = max;
		foldTree(testState, &side, numIterations, C, heuristic, budget[side], ci_threshold, goldStandard);
		_DOM->destructRep(testState);
	}
	
	free(mmVals);
	return 0;
}



int executeBuildTrees(int argc, char* argv[]) {

	int s, side = max, numBestMoves, arg = 0;
	rep_t testState;

	/* defaults */
	_DOM = init_domain(MANCALA); //currently mancala
	int numGames = 10;
	rep_t dummy1= 0;
	int dummy2=0;
	int bestMoves[_DOM->getNumOfChildren(dummy1,dummy2)];
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
	
	//printf("Seed, Instance, GS_level,  CI_Threshold, iterations, H , GS_Move, GS_Move_Val, CIB_Parental_Percantage , CIB_Average_depth, CIB_Min_Depth, CIB_Tree_Depth, CIB_Move, CIB_Move_Val, CIB_GS_Move_Val, UCT_Move, UCT_Move_Val, UCT_GS_Move_Val, Parental_Percantage , Average_depth, Min_Depth, Tree_Depth, CI_Move, CI_Move_Val, CI_GS_Move_Val, MMF_Move, MMF_Move_Val, MMF_GS_Val\n");
	
	//Header for tree building algorithms
	printf("Seed, Instance, GS_level,  CI_Threshold, Iterations, H , GS_Move, GS_Move_Val,");
        printf(" CIB_Parental_Percantage , CIB_Average_depth, CIB_Min_Depth, CIB_Tree_Depth,");
	printf(" CIB_Move, CIB_Move_Val, CIB_GS_Move_Val,");
        printf(" WMM_Parental_Percantage , WMM_Average_depth, WMM_Min_Depth, WMM_Tree_Depth,");
	printf(" WMM_Move, WMM_Move_Val, WMM_GS_Move_Val,");
	printf(" MMB_Move, MMB_Move_Val, MMB_GS_Move_Val, UCT_Move, UCT_Move_Val, UCT_GS_Move_Val,");
       	printf(" VAR_Move, VAR_Move_Val, VAR_GS_Move_Val\n");

	int children_count = _DOM->getNumOfChildren(dummy1,dummy2);
	double *mmVals = (double*)calloc(children_count, sizeof(double));
	for (s = 0; s < numGames; s++) {
		srandom(seed+s);
		testState = _DOM->allocate();
		_DOM->generateRandomStart(testState,&side);
		//Seed, Instance, GS_level,  CI_Threshold, iterations, h
		printf("%d, %d, %d, %d, %d, h%d", seed+s, s, gs_level, ci_threshold, numIterations, hfunc);
		//Make minmax move to depth 16
		side = max;
		makeMinmaxMove(testState, &side, gs_level , heuristic, budget[side], randomTieBreaks, noisyMM, bestMoves, &numBestMoves, &termPercentage, mmVals);
		int goldStandard = maxVal(mmVals, children_count );
		//GS_Move, GS_Move_Val,
		printf(" , %d, %f, ", goldStandard , mmVals[goldStandard]);
		side = max;
		buildTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard, CI);
		side = max;
		buildTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard, WEIGHTED_MM);
		side = max;
		buildTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard, MINMAX);
		side = max;
		buildTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard, AVERAGE);
		side = max;
		buildTree(testState, &side, numIterations, C[side], heuristic, budget[side], ci_threshold, goldStandard, VARIANCE);
		printf("\n");
		_DOM->destructRep(testState);
	}
	
	free(mmVals);
	return 0;
}

void buildTree(rep_t rep, int *side, int numIterations, double C, heuristics_t heuristic, int budget, int ci_threshold, int goldStandard, int backOp) {
		int i;
		rep_t dummy1=0;
		int dummy2=0;
		int count_children = _DOM->getNumOfChildren(dummy1,dummy2);
		double vals[count_children];
		treeNode* tree = buildUCTTree(rep, side, numIterations, C, heuristic, budget, backOp, ci_threshold);
		for (i = 1; i <count_children; i++) {
			if (tree->children[i]) { // if this child was explored
				vals[i-1]= tree->children[i]->scoreSum / (double)tree->children[i]->n;
				if (*side == min)
					 vals[i-1]= -vals[i-1];
			} else {
				assert(i>0);
				vals[i-1] = -1 * INF;
			}
		}
		
		freeTree(tree);
		int bestMove = maxVal(vals,count_children);
		printf(", %d, %f, %f ", bestMove, vals[bestMove], vals[goldStandard]); 
}

/**
 * Builds the UCT tree 
 * */
treeNode* buildUCTTree(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget, int backOp, int ci_threshold) {
	int i;
	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0;
	treeNode* rootNode = createRootNode(side,rep);
	if (C==9999){
		for (i = 0; i < numIterations; i++)
			uctRecurseRandom(rootNode, C, heuristic, budget, backOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
	}else{
		for (i = 0; i < numIterations; i++)
			uctRecurse(rootNode, C, heuristic, budget, backOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
	}
	if (backOp == CI || backOp == WEIGHTED_MM) {
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
	treeNode* tree = buildUCTTree(rep, side, numIterations, C, heuristic, budget, AVERAGE, INF);
	double uct_value = tree->scoreSum/tree->n ;
	double mm_value  = minmaxUCT(tree);
//	double cdp_value = cdp_tree_value(tree,t_test);
	double cdp_value = cdp_tree_value(tree,variance_test);
	printf("%f, ", uct_value); 
	printf("%f, ", cdp_value ); 
	printf("%f\n ", mm_value ); 
//	const int children_count = _DOM->getNumOfChildren(rep,*side); 
//	double* uct_vals = (double*)calloc(children_count,sizeof(double));
//	if (uct_vals==0){puts(" uct_vals not allocated ");}
//	double* uct_mm_vals = (double*)calloc(children_count,sizeof(double));
//	if (uct_mm_vals==0){puts(" uct_mm_vals not allocated ");}
//	double* uct_cdp_vals = (double*)calloc(children_count,sizeof(double));
//	if (uct_cdp_vals==0){puts(" uct_cdp_vals not allocated ");}
//
//	for (i = 1; i < children_count; i++) {
//		//printf(" i=%d \n", i); 
//		if (tree->children[i]) { // if this child was explored
//			//printf("assigning to i=%d, ", i-1); 
//			uct_vals[i-1]= tree->children[i]->scoreSum / (double)tree->children[i]->n;
//			uct_mm_vals[i-1]= minmaxUCT(tree->children[i]);
//			//vci *VCI = vciMinmaxUCT(tree->children[i], ci_threshold,&parentCIWin,&parentCITotal,&avgDepth,&minDepth,&treeDepth); // do a minmax backup this child
//			//uct_ci_vals[i-1] = weightedMM(tree->children[i],heuristic);
//			uct_cdp_vals[i-1] = cdp_tree_value(tree->children[i]);
//			//free(VCI);
//			
//		}else {
//			assert(i>0);
//			//printf("assigning to i=%d, ", i-1); 
//			uct_vals[i-1]=uct_mm_vals[i-1]=uct_cdp_vals[i-1] = -1*INF;
//		}
//		// negamax
//		if (*side == min){
//			uct_vals[i-1]= -uct_vals[i-1];
//			uct_mm_vals[i-1]= -uct_mm_vals[i-1];
//			uct_cdp_vals[i-1]= -uct_cdp_vals[i-1];
//		}
//	//printf(" **i=%d \n", i); 
//	}
//	int bestMoveOfUct = maxVal(uct_vals,children_count-1);
//	int bestMoveOfUct_mm = maxVal(uct_mm_vals,children_count-1);
//	int bestMoveOfUct_cdp = maxVal(uct_cdp_vals,children_count-1);
//	free(uct_vals);
//	free(uct_mm_vals);
//	free(uct_cdp_vals);
	freeTree(tree);
}


