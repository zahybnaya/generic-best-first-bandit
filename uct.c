/**
 *
 * UCT implementationa
 * Scoresum of node - sum of rewards / CI keeps "real score sum "  
 * */
#include "common.h"
#include "domain.h"
#include "phi.c"
#define SIMPLE_REGRET_UCT 0 //set to 1 to enable a different algorithm (see minimizing simple regret in MCTS)
#define z975 1.96

extern DOM* _DOM; 
extern int debuglog;
extern int logIteration;
extern int log_this_iteration;

// File scope globals
static short dotFormat = false; // determines if search tree is printed out
static int id = 0; // used to determine next node id to assign
static int mm_Counter; // for counting nodes

/* This is a node in the UCT tree. */
typedef struct node {
	double scoreSum; // stores the value of this node times n. didnt change the name in order not to break old code.
	int n; // tracks the visit count
	double M2; // variance of rewards == M2 / (n -1)
	double ci; //confidence interval in the score
	double realScoreSum; // this is the sum of total rewards gone through this node.
	int id; // used for graph visualization purposes
	rep_t rep; // generic representation of the state
	int side; // side on move at this board position
	int depth;
	struct node** children; /* pointers to the children of this node -- note that index 0 remains
				   unused (which is reserved for the store), so we have consistent move
				   indexing/numbering */
} treeNode;

typedef struct _child{int index; double value; double SD;} child_data;

static int comparChildData (const void* p1, const void* p2){
	child_data* c1 =(child_data*)p1;
	child_data* c2 =(child_data*)p2;
	if(c1->value > c2->value){
		return -1;
	}
	if(c1->value < c2->value){
		return 1;
	}
	return 0;

}
/* Routine to free up UCT tree */
static void freeTree(treeNode* node) {
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
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
/*
 * probability that child i is greater than child j
 * We assume that i and j are normally distributed so we perform the following:
 * We define X = Xi - Xj 
 * We return 1-P(X<=0)  which is 1-Phi(Xi-Xj/Sdi+sdj)
 * */
static double getProbForGreater(int i,int j, treeNode* node){
	double xi = node->children[i]->scoreSum/node->children[i]->n;
	double xj = node->children[j]->scoreSum/node->children[j]->n;
	if ((xi == MIN_WINS && node->side == min) || (xi == MAX_WINS && node->side == max) ){
		return 1;
	}
	if ((xj == MIN_WINS && node->side == min) || (xj == MAX_WINS && node->side == max) ){
		return 0;
	}
	double sdi=0.5,sdj=0.5;
	if(node->children[i]->n >1)
		sdi = node->children[i]->M2/(double)(node->children[i]->n - 1);
	if(node->children[j]->n >1)
		sdj = node->children[j]->M2/(double)(node->children[j]->n - 1);
	double s = (sdi+sdj);
	s = s>0?s:0.1;
	//printf("xi:%f, xj:%f, sdi:%f, sdj:%f \n",xi,xj,sdi,sdj);
	assert(s>0);
	double ret = phi((xi-xj)/s);
	ret = node->side==max?ret:(1-ret);
	//printf("side:%d phi(%f)=%f\n",node->side,(xi-xj)/s,ret);
  return ret;
}
/*
 * Returns the probability that i is the maximal child of node 
 *
 * */
static double getProbForMaximialChild(int mi, treeNode* node) {
	int i;
	double answer = 1;
	treeNode* child = node->children[mi];
	if (!child)
		return 0;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	  if (i==mi||!node->children[i]) { 
		  continue;
	  }
	  answer*=getProbForGreater(mi,i,node);
	}
	return answer;
}

/* Weighted Minimax*/
static double weightedMM(treeNode* node,heuristics_t heuristic) {
	int i;
	double val;
	double bestScore = 0;
	char s[1512],b[256]; 
	sprintf(s,"*side:%d visits:%d depth:%d ",node->side,node->n,node->depth);
	if ((val = _DOM->getGameStatus(node->rep)) != INCOMPLETE){
		if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5)) {
			val = (val/MAX_WINS);
		}
		return val; // return something from the set {MIN_WINS, DRAW, MAX_WINS}
	}
	if (node->n == 1) {
	  assert((node->scoreSum > MIN_WINS) && (node->scoreSum < MAX_WINS)); 
	  return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
	}
	double ttlPp=0;
	double rttlp=0;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	  if (node->children[i]) { // only descend if child exists
	    ttlPp+=getProbForMaximialChild(i,node); 
	  }
	}
	assert(ttlPp>0);
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	  if (node->children[i]) { // only descend if child exists
	    val = weightedMM(node->children[i], heuristic); 
	    double uctVal = node->children[i]->scoreSum/node->children[i]->n;
	    double uctProb = node->children[i]->n/(double)node->n;
	    double prob = getProbForMaximialChild(i,node)/ttlPp;
	    rttlp+=prob;
	    bestScore+=prob*val;
	    sprintf(b,"Child%d,v:%2.3f,uv:%2.3f,p:%2.3f,up:%2.3f,",i,val,uctVal,prob,uctProb);
	    strcat(s,b);
	  }
	}
	sprintf(b,"scr:%2.3f,scru:%2.3f,ttlp:%2.3f,one:%2.2f*\n",bestScore,node->scoreSum/(double)node->n,ttlPp,rttlp);
	strcat(s,b);
//	puts(s);

	return bestScore;
}


/* Invoked by uctRecurse to decide which child of the current node should be expanded */
static int selectMove(treeNode* node, double C, int isSimpleRegret) {
	int i;
	double qhat;
	double score;
	int numBestMoves = 0;
	double bestScore;
	int bestMoves[_DOM->getNumOfChildren()]; //This should be ok when compiled with c99+ 
	/*
	 * The multiplier is used to set the sign of the exploration bonus term (should be negative
	 for the min player and positive for the max player) i.e. so that we correctly compute
	 an upper confidence bound for Max and a lower confidence bound for Min */
	double multiplier = (node->side == max) ? 1 : -1;

	for (i = 1; i < _DOM->getNumOfChildren(); i++) { // iterate over all children
		if(!_DOM->isValidChild(node->rep, node->side, i)) { // if the i^th move is illegal, skip it
			continue;
		}
		// If the i^th child has never been visited before, select it first, before any other children are revisited
		if (node->children[i] == NULL || node->children[i]->n == 0)
			return i;

		// Otherwise, compute this child's UCB1 index (will be used to pick best child if it transpires that all
		// children have been visited at least once)
		qhat = node->children[i]->scoreSum / (double)node->children[i]->n;  // exploitation component (this is the average utility)
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
  double mean = node->realScoreSum / (double)n;
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
	double bestScore;
	double score;
	assert(node != NULL); // should never be calling uctRecurse on a non-existent node
	if ((ret = _DOM->getGameStatus(node->rep))!= INCOMPLETE) {
		fflush(stdout);
		
		if (_DOM->dom_name == SAILING)
		  ret = 0;
		
		// This is a terminal node (i.e. can't generate any more children)
		// If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
		// those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
		// which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
		// node values. If we are using engineered heuristics, then no rescaling is necessary.
		if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5)) {
			ret /= MAX_WINS; // rescale
		}
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
		node->children[move]->children =(treeNode**)  calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
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
	if (backupOp == AVERAGE) { // use averaging back-up
		updateStatistics(node, ret);
	}
	else if (backupOp == MINMAX) { // use minimaxing back-up
		(node->n)++;
		bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
		
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i]) { // if child exists, is it the best scoring child?
				score = node->children[i]->scoreSum / (double)node->children[i]->n;
				if (   ((node->side == max) && (score > bestScore))
						|| ((node->side == min) && (score < bestScore)))
					bestScore = score;
			}
		}
		node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
	}else if (backupOp == WEIGHTED_MM) { 
		updateStatistics(node,ret);
		double ttlPp=0;
		double nodeScore=0;
		double nodeP,nodeVal;
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i]) { // if child exists, is it the best scoring child?
				nodeP=getProbForMaximialChild(i,node);
				ttlPp+=nodeP;
				nodeVal = node->children[i]->scoreSum / (double)node->children[i]->n;
				nodeScore+=(nodeVal*nodeP);
			}
		}
		node->scoreSum = (node->n) * (nodeScore/ttlPp); 
	} else if (backupOp == CI) {
		if (node->n < ci_threshold) {
		  updateStatistics(node, ret);
		  return ret;
		}
		
		bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
		double bestCI = INF;
		
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i] && node->children[i]->n >= ci_threshold) { // if child exists, is it the best scoring child?
				score = node->children[i]->scoreSum / (double)node->children[i]->n;
				if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore))){
					bestScore = score;
					bestCI = node->children[i]->ci;
				}
			}
		}

		// Calculate the CI length for the current node.
		double variance = node->M2 / (double)(node->n - 1);
		node->ci = 2 * z975 * sqrt(variance / (double)node->n);
		
		(*parentCITotal)++;
		if (bestCI < node->ci) {
		  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
		  node->ci = bestCI;
		  updateStatistics(node, ret);
		  node->scoreSum -= ret; //updateStatistics also adds the reward to the scoreSum which shouldn't happen in this case.
		  
		  (*parentCIWin)++;
		  *avgDepth = ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
		  if(*minDepth > node->depth)
			*minDepth = node->depth;
		  
		} else {
		  node->scoreSum = node->realScoreSum; // reset score to average of current node
		  updateStatistics(node, ret);
		}
	}
	else if (backupOp == VARIANCE_ALL) {
		if (node->n < ci_threshold) {
		  updateStatistics(node, ret);
		  return ret;
		}
		child_data children_data[_DOM->getNumOfChildren()];
		int numOfBestChildren=0;
		double nodeScore = node->scoreSum / (node->n),childSd ;
		nodeScore=node->side==max?nodeScore:-nodeScore;
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i] && node->children[i]->n >= ci_threshold) { // if child exists, is it the best scoring child?
				score = node->children[i]->scoreSum / (double)node->children[i]->n;
				score = node->side==max?score:-score;
				childSd = node->children[i]->ci;
				children_data[numOfBestChildren++] =(child_data){i, score, childSd};
			}
		}
		qsort(children_data,numOfBestChildren,sizeof(child_data),comparChildData);
		for(i=1;i<numOfBestChildren;i++){
			assert(children_data[i].value<=children_data[i-1].value);
		}
		node->ci = sqrt(node->M2 / (double)(node->n - 1));
		for (i=0;i<numOfBestChildren;i++){
			(*parentCITotal)++;
			if (children_data[i].value>nodeScore && children_data[i].SD < node->ci){
				nodeScore=(node->n) * children_data[i].value; 
				nodeScore=(node->side==max)?nodeScore:-nodeScore;//reversing
				node->scoreSum = nodeScore;
				node->ci = children_data[i].SD ;
				updateStatistics(node, ret);
				node->scoreSum -= ret; //updateStatistics also adds the reward to the scoreSum which shouldn't happen in this case.
				(*parentCIWin)++;
				*avgDepth = ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
				if(*minDepth > node->depth){
					*minDepth = node->depth;
				}
				if(i>4)printf("child%d\n",i);
				return ret;
			}
		}
		node->scoreSum = node->realScoreSum; // reset score to average of current node
		updateStatistics(node, ret);
	}
	else if (backupOp == VARIANCE) {
		if (node->n < ci_threshold) {
			updateStatistics(node, ret);
			return ret;
		}

		bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
		double bestSD = INF;

		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i] && node->children[i]->n >= ci_threshold) { // if child exists, is it the best scoring child?
				score = node->children[i]->scoreSum / (double)node->children[i]->n;
				if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore))){
					bestScore = score;
					bestSD = node->children[i]->ci;
				}
			}
		}

		// Calculate the standard deviation of the current node.
		node->ci = sqrt(node->M2 / (double)(node->n - 1));
		
		(*parentCITotal)++;
		if (bestSD < node->ci) {
		  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
		  node->ci = bestSD;
		  updateStatistics(node, ret);
		  node->scoreSum -= ret; //updateStatistics also adds the reward to the scoreSum which shouldn't happen in this case.
		  
		  (*parentCIWin)++;
		  *avgDepth = ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
		  if(*minDepth > node->depth)
			*minDepth = node->depth;
		  
		} else {
		  node->scoreSum = node->realScoreSum; // reset score to average of current node
		  updateStatistics(node, ret);
		}
	}
	else { // shouldn't happen
		puts("Invalid back-up operator!");
		exit(1);
	}	
	return ret;
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
	if(debuglog)puts("UCT");
	// Create the root node of the UCT tree; populate the board and side on move fields
	rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = *side;
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));

	// Run specified number of iterations of UCT
	int parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	double avgDepth=0;
	for (i = 0; i < numIterations; i++){
		//printf("ITERATION %d\n", i);
		uctRecurse(rootNode, C, heuristic, budget, backupOp, true, ci_threshold, &parentCIWin, &parentCITotal, &avgDepth, &minDepth, &treeDepth);
	}

	// Now look at the children 1-ply deep and determing the best one (break ties
	// randomly)
	for (i = 1; i < _DOM->getNumOfChildren(); i++) { // for each move
		if (!_DOM->isValidChild(rep,*side, i))
			continue;
		if (!rootNode->children[i]) // this node was not created since # iterations was too small
			continue;
		
		val = rootNode->children[i]->scoreSum / (double)rootNode->children[i]->n;
		//printf("UCT:%d:%f/%d\n",i,rootNode->children[i]->scoreSum,rootNode->children[i]->n);
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


/*
 *  Creates the root of the tree
 * */
treeNode* createRootNode(int* side, rep_t rep){
	// Create the root node of the UCT tree; populate the board and side on move fields
	treeNode* rootNode;
	rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = *side;
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
	return rootNode;
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
	rootNode =(treeNode*) calloc(1, sizeof(treeNode));
	rootNode->rep=_DOM->cloneRep(rep);
	rootNode->side = side;
	rootNode->id = ++id;
	rootNode->children =(treeNode**) calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));

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
	bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;

	// Is this a terminal node?
	if ((val = _DOM->getGameStatus(node->rep)) != INCOMPLETE)
		return val; // return something from the set {MIN_WINS, DRAW, MAX_WINS}

	// Is this a leaf node? (can determine this by looking at the UCT visit count to this node)
	if (node->n == 1) {
	  assert((node->scoreSum > MIN_WINS) && (node->scoreSum < MAX_WINS)); // make sure heuristic is bounded by terminal node values
	  return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
	}

	// Otherwise, we are at an internal node, so descend recursively
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
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
	if (_DOM->getGameStatus(node->rep) != INCOMPLETE) {
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
		bestVCI->score = node->scoreSum / (double)node->n;
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
	double bestChildScore = node->side==max? MIN_WINS : MAX_WINS;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
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
	bestVCI->score = node->scoreSum / (double)node->n;
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
	rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = *side;
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));

	// Run specified number of iterations of UCT
	for (i = 0; i < numIterations; i++)
		uctRecurse(rootNode, C, heuristic, budget, AVERAGE, true, INF, &parentCIWin,&parentCITotal,&avgDepth,&minDepth,&treeDepth);

	parentCIWin=0,parentCITotal=0,minDepth=INF,treeDepth=0;
	
	// Now minimax the tree we just built (we minimax starting from each child)
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
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
