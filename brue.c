#include "common.h"
#include "domain.h"
#define HORIZON 20 //TODO this is domain depandent, push it to DOM

/* This is a node in the UCT tree. */
typedef struct node {
	double scoreSum; // stores the sum of the rewards of the episodes that have gone through this node
	int n; // tracks the visit count
	rep_t rep; // generic representation of the state
	int side; // side on move at this board position
	struct node* parent;
	struct node** children; /* pointers to the children of this node -- note that index 0 remains*/
} treeNode;

typedef enum policy {EXPLORATION, EXPLOITATION} POLICY;
extern DOM* _DOM; 

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
 *  Create a node
 */
treeNode* createNode(rep_t rep, int side, treeNode* parent){
	treeNode* rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = side;
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(rep, side), sizeof(treeNode*));
	rootNode->parent = parent;
	return rootNode;
}
/**
 * find the switching point 
 */
int getSwitchingPoint(int i, int horizon) {
	return i%horizon;	
}


int selectBestMove(int* bestMoves, int numBestMoves) {
	return bestMoves[random() % numBestMoves]; // pick the best move (break ties randomly)
}

/**
 *  best moves 
 */
void findBestMoves(treeNode* root, int* bestMoves, int* numBestMoves) {
	int i;
	double val, bestScore;
	for (i = 1; i < _DOM->getNumOfChildren(root->rep, root->side); i++) { // for each move
		if (!_DOM->isValidChild(root->rep,root->side, i))
			continue;
		if (!root->children[i]) // this node was not created since # iterations was too small
			continue;
		val = root->children[i]->scoreSum / (double)root->children[i]->n;
		// If this was min's move, negate the utility value (this makes things a little cleaner
		// as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
		val = (root->side == min) ? -val : val;
		// If this is the first child, or the best scoring child, then store it
		if ((*numBestMoves == 0) || (val > bestScore)) {
			bestMoves[0] = i;
			bestScore = val;
			*numBestMoves = 1;
		}
		else if (val == bestScore) // child ties with currently best scoring one; store it
			bestMoves[(*numBestMoves)++] = i;
	}
}


/**
 * returns the first valid move
 */
int validRandomMove(treeNode* current)
{
	int i,validCounter=0;
	int valids[_DOM->getNumOfChildren(current->rep, current->side)];
	for (i = 1; i < _DOM->getNumOfChildren(current->rep, current->side); i++) { // for each move
		if (_DOM->isValidChild(current->rep,current->side, i))
			valids[++validCounter]=i;
	}	
	return valids[random() % validCounter]; 
}
/**
 * returns the first valid move
 */
int validMove(treeNode* current)
{
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(current->rep, current->side); i++) { // for each move
		if (_DOM->isValidChild(current->rep,current->side, i))
			return i;
	}	
	assert(false);
	return -1;
}

/**
 * The exploitation policy for selecting the next node
 */
int selectMoveExploitation(treeNode* current){
	int bestMoves[_DOM->getNumOfChildren(current->rep, current->side)];
	int numBestMoves=0;
	findBestMoves(current,bestMoves,&numBestMoves);
	return selectBestMove(bestMoves,numBestMoves);
}

/**
 * The exploration policy for selecting the next node
 */
treeNode* nextNode(treeNode* current, POLICY p) {
	int ret,move;
	if ((ret = _DOM->getGameStatus(current->rep))!= INCOMPLETE) {
		return NULL;
	}
	if (current->n == 0){
		move = validMove(current);	
		rep_t newRep = _DOM->cloneRep(current->rep);
		int newSide = current->side; 
		_DOM->makeMove(newRep,&newSide, move); 
		current->children[move] = createNode(newRep,newSide,current);
		return current->children[move];
	}
	if (p == EXPLORATION){
		move = validRandomMove(current);
	} else {
		move =  selectMoveExploitation(current);
	}
	return current->children[move];
}

/**
 * performs the backpropagations
 */
void backpropagate(treeNode* leaf, double reward)
{
	while (leaf->parent != NULL){
		(leaf->n)++;
		leaf->scoreSum += reward;
		leaf = leaf->parent;
	} 
}
/**
 *
 * get reward from leaf
 */
double getReward(treeNode* leaf, heuristics_t heuristic, int budget){
	int ret;
	if ((ret = _DOM->getGameStatus(leaf->rep))!= INCOMPLETE) {
		if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5))
			ret /= MAX_WINS; // rescale
	}
	else if (leaf->n == 0) { 
		ret = heuristic(leaf->rep , leaf->side, budget);
	}
	return ret;
}


/**
 *  Performs the iteration of BRUE
 */
void performIteration(treeNode* root,  int switchingPoint, heuristics_t heuristic, int budget){
	int level = 0; 
	treeNode* n = root, *leaf = NULL;
	while (n != NULL && n->n>0){
		leaf = n;
		POLICY p = level < switchingPoint? EXPLORATION : EXPLOITATION;
		n = nextNode(n,p);	
		level++;
	}
	double reward = getReward(leaf, heuristic, budget);
	backpropagate(leaf, reward);
} 

/**
 * Makes the BRUE move
 */
int makeBrueMove(rep_t rep, int *side, int numIterations, heuristics_t  heuristic, int* bestMoves, int* numBestMoves, int budget) {
	const int horizon = HORIZON;
	int i, switchingPoint;
	treeNode* root = createNode(rep,*side, NULL);
	*numBestMoves = 0; // reset size of set of best moves
	for (i = 0; i < numIterations; i++) {
		switchingPoint = getSwitchingPoint(i,horizon); 
		performIteration(root,switchingPoint, heuristic, budget);
	}
	findBestMoves(root, bestMoves, numBestMoves);
	freeTree(root); 
	return selectBestMove(bestMoves,*numBestMoves);
}




