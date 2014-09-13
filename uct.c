/**
 *
 * UCT implementationa
 * Scoresum of node - sum of rewards / CI keeps "real score sum "  
 * */
#include "common.h"
#include "domain.h"
#include "uct.h"

// File scope globals
static short dotFormat = false; // determines if search tree is printed out
static int id = 0; // used to determine next node id to assign
static int mm_Counter; // for counting nodes

/* Routine to free up UCT tree */
static void freeTree(treeNode* node) {
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (node->children[i]) {
			freeTree(node->children[i]);
			node->children[i] = NULL;
		}
	}
	_DOM->destructRep(node->rep);
	free(node->children);
	free(node);
}

/**
 * Exploitation term acording to the uct formula 
 */ 
static double uctExploration(double multiplier, double C, treeNode* node, int i){
	return (multiplier * C) * sqrt(log(node->n) / (double)node->children[i]->n); // add exploration component
}

/**
 * Exploitation term acording to the simple regret uct formula 
 */ 
static double uctExplorationSimpleRegret(double multiplier, double C, treeNode* node, int i){
	return 	sqrt((multiplier * C) * sqrt(node->n)/(double)node->children[i]->n); // add exploration component
}

double avgRewards(treeNode *node) {
  return node->realScoreSum / (double)(node->n);
}

double assignedScore(treeNode *node) {
  return node->scoreSum / (double)(node->n);
}

/* Invoked by uctRecurse to decide which child of the current node should be expanded */
static int selectMove(treeNode* node, double C, int isSimpleRegret) {
	int i;
	double qhat;
	double score;
	int numBestMoves = 0;
	double bestScore;
	int bestMoves[_DOM->getNumOfChildren(node->rep, node->side)]; //This should be ok when compiled with c99+ 
	/*
	 * The multiplier is used to set the sign of the exploration bonus term (should be negative
	 for the min player and positive for the max player) i.e. so that we correctly compute
	 an upper confidence bound for Max and a lower confidence bound for Min */
	double multiplier = (node->side == max) ? 1 : -1;

	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) { // iterate over all children
		if(!_DOM->isValidChild(node->rep, node->side, i)) { // if the i^th move is illegal, skip it
			continue;
		}
		// If the i^th child has never been visited before, select it first, before any other children are revisited
		if (node->children[i] == NULL || node->children[i]->n == 0)
			return i;

		// Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
		// children have been visited at least once)
		qhat = assignedScore(node->children[i]);  // exploitation component (this is the average utility)
		if (_DOM->dom_name == SAILING)
		  qhat += actionCost_sailing(node->rep, i);
		
		if (isSimpleRegret){
			score = qhat + uctExplorationSimpleRegret(multiplier,C,node,i);
		} else {
			score = qhat + uctExploration(multiplier,C,node,i);
		}
		// Negamax formulation -- since min(s1,s2,...) = -max(-s1,-s2,...), negating the indices when it
		// is min's turn means we can always just take the maximum
		score = (node->side == min) ? -score : score;
		if(debuglog)printf("Score: %4.2f*(SQRT(LOG(%d)/%d)+=%4.2f/%d)=%4.2f\n",C,node->n,node->children[i]->n,node->children[i]->scoreSum,node->children[i]->n,score);
		// If this is either the first child, or the best scoring child, store it
		if ((numBestMoves == 0) || (score > bestScore)) {
			bestMoves[0] = i;
			bestScore = score;
			numBestMoves = 1;
		}
		else if (score == bestScore) // if this child ties with the best scoring child, store it
			bestMoves[numBestMoves++] = i;
	}
	// Return the next child to explore (break ties randomly -- FIXED tie-breaking for debuging purposes)
	int chosenBestMove = bestMoves[0];
	/*log*/
	if(logIteration){
		printf("%f: ",bestScore);
	}

	return chosenBestMove;
}

/**
 * Update visits, score sum, and variance of rewards of a node.
 */
void updateStatistics(treeNode *node, double sample) {
  if (node->n == 0) {
    node->n++;
    node->scoreSum = sample;
    node->realScoreSum = sample;
    node->ci = INF;
    return;
  }
  
  int n = node->n;  
  double M2 = node->M2;
  double mean = avgRewards(node);
  double delta = sample - mean;
  
  n++;
  mean = mean + delta / (double)n;
  M2 = M2 + delta * (sample - mean);
  
  node->n = n;
  node->scoreSum += sample;
  node->realScoreSum += sample;
  node->M2 = M2;
}

/* Recursively constructs the UCT search tree */
static double uctRecurse(treeNode* node, double C, heuristics_t heuristic, int budget, int backupOp , int isRoot, int ci_threshold, int* parentCIWin, int* 						parentCITotal, double* avgDepth, int* minDepth, int* treeDepth) {
	double ret;
	int move,i;
	//C = fabs(assignedScore(node));
	assert(node != NULL); // should never be calling uctRecurse on a non-existent node
	if ((ret = _DOM->getGameStatus(node->rep))!= _DOM->incomplete) {
		fflush(stdout);
		
		if (_DOM->dom_name == SAILING)
		  ret = 0;
		
		// This is a terminal node (i.e. can't generate any more children)
		// If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
		// those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
		// which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
		// node values. If we are using engineered heuristics, then no rescaling is necessary.
		if ((_DOM->dom_name == MANCALA &&
				       	((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5))) || (_DOM->dom_name == GGP && heuristic == _DOM->hFunctions.h2))
			ret /= _DOM->max_wins; // rescale
		assert(ret>=-1 && ret <=1);

		if ((dotFormat) && (node->n == 0)) // on first visit to a terminal node, color it red
			printf("n%d [color=\"red\"];", node->id);
		
		// Update node score / count and return
		updateStatistics(node, ret);
		node->ci = 0;
		return ret;
	}
	else if (node->n == 0) { // not a terminal node
		// This is the first visit to this state -- we estimate its utility according to the user-specified
		// heuristic (playouts or other heuristics)
		ret = heuristic(node->rep , node->side, budget);
		// Update node score / count and return
		updateStatistics(node, ret);
		return ret;
	}

	// We are at an internal node that has been visited before; descend recursively
	// Use selectMove to pick which branch to explore.
	//TODO handle chance node diffrently (domain independant)
	if (_DOM->dom_name == SAILING && isChanceNode_sailing(node->rep) == true) {
	  move = selectMoveStochastic_sailing(node->rep);
	} else {
	  move = selectMove(node, C, SIMPLE_REGRET_UCT ? isRoot : false);
	}

	// If this board does not have a node already created for it, create one for it now
	if (node->children[move] == NULL) {
		mm_Counter++;
		node->children[move] = (treeNode*) calloc(1, sizeof(treeNode));
		node->children[move]->rep = _DOM->cloneRep(node->rep);// copy over the current board to child
		node->children[move]->side = node->side; // copy over the current side on move to child
		node->children[move]->depth = node->depth + 1;
		if (dotFormat) { // we are visiting a node for the first time -- assign it an id and print the edge leading to it
			node->children[move]->id = ++id;
			printf("n%d -> n%d;\n", node->id, id);
		}
		// From the current board, we make the move recommended by selectMove() -- the resulting game state
		// is what is stored in the child node
		_DOM->makeMove(node->children[move]->rep, &(node->children[move]->side), move);
		node->children[move]->children = (treeNode**)calloc(_DOM->getNumOfChildren(node->children[move]->rep, node->children[move]->side), sizeof(treeNode*));
		if(debuglog)printf("Created a node for move %d\n",move);
		
		if (node->depth > *treeDepth)
			*treeDepth = node->depth;
	}

	// Descend recursively
	ret = uctRecurse(node->children[move], C, heuristic, budget, backupOp, false, ci_threshold, parentCIWin, parentCITotal, avgDepth, minDepth, treeDepth);

	//In a stochastic domain
	//Update ret to include the cost of getting to the child node from this parent.
	if (_DOM->dom_name == SAILING && isChanceNode_sailing(node->rep) == false)
	  ret += actionCost_sailing(node->rep, move);
	
	// Update score and node counts and return the outcome of this episode
	if (_DOM->dom_name == SAILING && isChanceNode_sailing(node->rep) == true && backupOp != AVERAGE) {
	  //in chance nodes we need to average all children, and only need to recalculate if we're not using average backpropagation
	  updateStatistics(node, ret);
	  
	  if (node->n < ci_threshold)
	    return ret;
	  
	  node->scoreSum = 0;
	  node->ci = 0;
	  for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
	    if (node->children[i]) {
	      node->scoreSum += node->children[i]->scoreSum;
	      node->ci += pow(node->children[i]->ci, 2);
	    }
	  }
	  node->ci = sqrt(node->ci);
	  
	  return ret;
	}
	if (backupOp == AVERAGE){ // use averaging back-up
		updateStatistics(node, ret);}
	else if (backupOp == WILCOXON) {
		wilcoxon_backup(node, ret);}
	else if (backupOp == COULOM) 
		coulom(node, ret);
	else if (backupOp == MINMAX) 
		minmax_backup(node);
	else if (backupOp == WEIGHTED_MM)
		weighted_mm_backup(node, ret);
	else if (backupOp == CI)
		ci_backup(node, ret, ci_threshold);
	else if (backupOp == VARIANCE_ALL)
		subset_backup(node, ret, ci_threshold, standardDeviation);
	//	subset_backup_agg(node,ret,ci_threshold, standardDeviationAggregated);
	else if (backupOp == VARIANCE)
		variance_backup(node, ret, ci_threshold);
	else if (backupOp == SIZE)
		size_backup(node, ret, ci_threshold);
	else if (backupOp == CI_ALL)
		subset_backup(node, ret, ci_threshold, confidenceInterval);
	else { // shouldn't happen
		puts("Invalid back-up operator!");
		exit(1);
	}	
	return ret;
}

/*
 *  Creates the root of the tree
 * */
static treeNode* createRootNode(int* side, rep_t rep) {
	// Create the root node of the UCT tree; populate the board and side on move fields
	treeNode* rootNode;
	rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = *side;
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(rootNode->rep, rootNode->side), sizeof(treeNode*));
	return rootNode;
}

/* Takes current board and side on move and runs numIterations of UCT with exploration
   bias of C. The outcome of the search is then used to make the best move (which is
   the value returned) */
int makeUCTMove(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic, int budget,
		int* bestMoves, int* numBestMoves, int backupOp, int ci_threshold, double *moveVals) {
	int i;
	double val;
	int bestMove = NULL_MOVE;
	double bestScore;
	treeNode* rootNode;
	*numBestMoves = 0; // reset size of set of best moves
	
	if (debuglog) puts("UCT");
	
	// Create the root node of the UCT tree; populate the board and side on move fields
	rootNode = createRootNode(side, rep);

	// Run specified number of iterations of UCT
	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0;
	for (i = 0; i < numIterations; i++){
		//printf("ITERATION %d\n", i);
		uctRecurse(rootNode, C, heuristic, budget, backupOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
	}

	// Now look at the children 1-ply deep and determing the best one (break ties
	// randomly)
	for (i = 1; i < _DOM->getNumOfChildren(rootNode->rep, rootNode->side); i++) { // for each move
		if (!_DOM->isValidChild(rep,*side, i)) {
			//printf("not valid\n");
			continue;
		}
		if (!rootNode->children[i]) { // this node was not created since # iterations was too small
			//printf("doesn't exist\n");
			continue;
		}
		
		val = assignedScore(rootNode->children[i]);
		if (_DOM->dom_name == SAILING)
		  val += actionCost_sailing(rootNode->rep, i);
		
		// If this was min's move, negate the utility value (this makes things a little cleaner
		// as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
		val = (*side == min) ? -val : val;
		
		if (moveVals != 0)
		  moveVals[i-1] = val;
		
		// If this is the first child, or the best scoring child, then store it
		if ((*numBestMoves == 0) || (val > bestScore)) {
			bestMoves[0] = i;
			bestScore = val;
			*numBestMoves = 1;
		}
		else if (val == bestScore) // child ties with currently best scoring one; store it
			bestMoves[(*numBestMoves)++] = i;
		if (verbose)
			printf("Move # %d -- Value %f, Count %d\n", i, ((*side == min) ? -val : val), rootNode->children[i]->n);
	}

	// We should have at least looked at one child
	assert(*numBestMoves != 0);
	//bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
	bestMove = bestMoves[0];

	if (verbose) {
		printf("Value of root node: %f\n", rootNode->scoreSum / (double)rootNode->n);
		printf("Best move: %d\n", bestMove);
	}

	// Clean up before returning
	freeTree(rootNode);

	return bestMove;
}

/* Performs the specified UCT search, while generating the structure of the search tree in dot format */
void genUCTTree(rep_t rep, int side, int numIterations, double C, heuristics_t heuristic, int budget) {
	treeNode* rootNode;
	int i;

	puts("digraph uct {");
	puts("root = n1;");
	puts("node [shape=\"point\"];");
	puts("edge [arrowhead=\"none\"];");
	puts("n1 [shape=\"box\", style=\"filled\", color=\"blue\", height=0.2, width=0.2, label=\"\"];");

	// Generate the body of the graph using specified settings
	dotFormat = true;
	assert(id == 0);  // for now, we only generate one graph per run

	// Create the root node of the UCT tree; populate the board and side on move fields
	rootNode = createRootNode(&side, rep);
	rootNode->id = ++id;

	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0;
	
	// Run specified number of iterations of UCT (this outputs the search tree)
	for (i = 0; i < numIterations; i++)
		uctRecurse(rootNode, C, heuristic, budget, AVERAGE, true, INF,&parentCIWin,&parentCITotal, &avgDepth, &minDepth,&treeDepth);

	dotFormat = false;

	printf("}\n");

	// Clean up
	freeTree(rootNode);

	return;
}

/* Recursively computes the minimax value of the UCT-constructed tree rooted at 'node' */
static double minmaxUCT(treeNode* node) {
	int i;
	double val;
	double bestScore;

	// Should never be reaching a non-existent node
	assert(node != NULL);

	// Initialize bestScore to a very unfavorable value based on who is on move
	bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;

	// Is this a terminal node?
	if ((val = _DOM->getGameStatus(node->rep)) != _DOM->incomplete)
		return val; // return something from the set {MIN_WINS, DRAW, MAX_WINS}

	// Is this a leaf node? (can determine this by looking at the UCT visit count to this node)
	if (node->n == 1) {
	  assert((node->scoreSum > _DOM->min_wins) && (node->scoreSum < _DOM->max_wins)); // make sure heuristic is bounded by terminal node values
	  return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
	}

	// Otherwise, we are at an internal node, so descend recursively
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
	  if (node->children[i]) { // only descend if child exists
	    val = minmaxUCT(node->children[i]); // compute minimax value of i^th child
	    if ((node->side == max) && (val > bestScore)) // maximizing level -- is this child score higher than the best so far?
	      bestScore = val;
	    else if ((node->side == min) && (val < bestScore)) // minimizing level -- is this child score lower than the best so far?
	      bestScore = val;
	  }
	}

	return bestScore;
}

//Value and confidence interval
typedef struct vci {
	double score;
	double ci; //length of confidence interval.
} vci;


static vci *vciMinmaxUCT(treeNode* node, int ci_threshold, int* parentCIWin,int* parentCITotal, double* avgDepth, int* minDepth,int* treeDepth) {
	assert(node != NULL);
	vci *currentVCI = NULL,*bestChild=NULL;
	int i;
	
	//Terminals returns CI=0
	if (_DOM->getGameStatus(node->rep) != _DOM->incomplete) {
		vci *bestVCI = (vci*)calloc(1, sizeof(vci));
		bestVCI->score = _DOM->getGameStatus(node->rep);
		bestVCI->ci = 0;
		if(node->depth>*treeDepth){
			*treeDepth = node->depth;
		}
		return bestVCI;
	}
	
	//below threshold return CI=INF(ignored)
	if (node->n < ci_threshold) {
		vci *bestVCI = (vci*)calloc(1, sizeof(vci));
		bestVCI->score = assignedScore(node);
		bestVCI->ci = INF;
		if(node->depth>*treeDepth){
			*treeDepth = node->depth;
		}
		return bestVCI;
	}

	// Calculate the CI length and score of node
	double variance = 0; 
	double nodeci = 0;
	if (node->n > 1) {
	       	variance = node->M2 / (double)(node->n - 1);
		nodeci = 2 * z975 * sqrt(variance / (double)node->n);
	}
	double bestChildScore = node->side==max? _DOM->min_wins : _DOM->max_wins;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (node->children[i]) { // only descend if child exists
			currentVCI = vciMinmaxUCT(node->children[i], ci_threshold,parentCIWin,parentCITotal,avgDepth,minDepth,treeDepth); 
			if ((((node->side == max) && (currentVCI->score >= bestChildScore )) || ((node->side == min) && (currentVCI->score <=bestChildScore )))
					&& currentVCI->ci < INF) { 
				bestChild = currentVCI;
				bestChildScore = currentVCI->score ;
			} else {
				free(currentVCI);
			}
		}
	}
	if(node->depth>*treeDepth){
		*treeDepth = node->depth;
	}
	(*parentCITotal)++;
	if (bestChild){
		(*parentCIWin)++;
		*avgDepth= ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
		if(*minDepth>node->depth){
			*minDepth = node->depth;
		}
		//if(node->n > ci_threshold){
		//	bestChild->ci=0;
		//}
		return bestChild;
	}
	vci *bestVCI = (vci*)calloc(1, sizeof(vci));
	bestVCI->score = assignedScore(node);
	bestVCI->ci = nodeci;
	return bestVCI;
}

/* Runs specified number of iteration of UCT. Then, runs minimax on the resulting UCT tree and returns the best move */
int makeMinmaxOnUCTMove(rep_t rep, int *side, int numIterations, double C,
		heuristics_t heuristic,
		int budget,
		int* bestMoves, int* numBestMoves, int ci_threshold, double *moveVals) {
	int i;
	double val;
	int bestMove = NULL_MOVE;
	double bestScore,avgDepth=0;
	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	treeNode* rootNode;
	
	*numBestMoves = 0; // reset size of set of best moves

	// Create the root node of the UCT tree; populate the board and side on move fields
	rootNode = createRootNode(side, rep);
	
	// Run specified number of iterations of UCT
	for (i = 0; i < numIterations; i++)
		uctRecurse(rootNode, C, heuristic, budget, AVERAGE, true, INF, &parentCIWin,&parentCITotal,&avgDepth,&minDepth,&treeDepth);

	parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	
	// Now minimax the tree we just built (we minimax starting from each child)
	for (i = 1; i < _DOM->getNumOfChildren(rootNode->rep, rootNode->side); i++) {
		if (rootNode->children[i]) { // if this child was explored
			if (ci_threshold <= 0) {
			  val = minmaxUCT(rootNode->children[i]);
			} else {
			  vci *VCI = vciMinmaxUCT(rootNode->children[i], ci_threshold,&parentCIWin,&parentCITotal,&avgDepth,&minDepth,&treeDepth); // 
			  val = VCI->score;
			  free(VCI);
			}
			
			// If this was min's move, negate the utility value (this makes things a little cleaner
			// as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
			val = (*side == min) ? -val : val;
			    
			if (moveVals != 0)
			  moveVals[i-1] = val;
			
			// If this is the first child, or the best scoring child, then store it
			if ((*numBestMoves == 0) || (val > bestScore)) {
				bestMoves[0] = i;
				bestScore = val;
				*numBestMoves = 1;
			}
			else if (val == bestScore) // child ties with currently best scoring one; store it
				bestMoves[(*numBestMoves)++] = i;

			if (verbose)
				printf("Move # %d -- Value %f\n", i, ((*side == min) ? -val : val));
		}
	}

	// We should have at least looked at one child
	assert(*numBestMoves != 0);

	//bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
	bestMove = bestMoves[0];

	if (verbose)
		printf("Best move: %d\n", bestMove);
	
	/*if (ci_threshold > 0) {
	  printf(",%f ," ,parentCIWin / (double)parentCITotal);
	}*/
	
	parentCIWin = 0;
	parentCITotal = 0;
	
	// Clean up when done
	freeTree(rootNode);

	return bestMove;
}

void printUctStats(){
	printf("** Total UCT nodes %d**\n",mm_Counter);
}
