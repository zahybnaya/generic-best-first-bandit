#include "common.h"
#include "type.h"
#include "type_reachability.h"

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

static void freeTypeSystems(type_system **type_systems) {
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(); i++)
		if (type_systems[i])
			type_systems[i]->destroy(type_systems[i]);

	free(type_systems);
}

static void  reportTypeStat(type_system* ts){
	double averageDepth =0; 
	double averageSide = 0;
	int n_types = ts->numTypes;
	int i = 0; 
	for (i =0;i<n_types;i++){
		averageDepth+=((type_sts *)ts->types[i])->root->depth;
		averageSide+=((type_sts *)ts->types[i])->root->side;
	}
	double avgValueForType = averageValueOfAllTypes(ts);
	assert(averageSide/n_types == 1.0 ||averageSide/n_types == 0.0 );
	printf("#visits: %d #types:%d averageDepth:%f averageValue:%f averageSide:%f\n",ts->visits, n_types, averageDepth/n_types,avgValueForType,averageSide/n_types );


}

/**
 * Is current defines a type? 
 * returns the index of the type or -1 if not
 * */
static int isTypeDefiner(treeNode* current, type_system* ts){
	int i = 0; 
	for (i = 0; i < ts->numTypes; i++) {
		if (((type_sts*)ts->types[i])->root == current){
			return i;
		}
	}
	return -1;
}
/**
 * selects a type by going UCT from the root. 
 */
static int selectTypeFromRoot(void *void_ts, double C, int side, int policy, int backupOp,double bolzmanConstant,double probWeight, treeNode* root) {
	treeNode* current = root;
	type_system* ts = (type_system*)void_ts;
	int type = -1;
	while(current){
		int nextMove = selectMove(current,C,false);
		current = current->children[nextMove];
		type = isTypeDefiner(current,ts);
		if (type){
			return type;
		}
	}
	assert(false);
	return -1;
}

//Invoked by bfbIteration to select a type out of the type system based on UCB1
static int selectType(void *void_ts, double C, int side, int policy, 
		int backupOp,double bolzmanConstant,double probWeight, int multiplyWithAverage, int isUseMinmaxRewards) {
	type_system *ts = (type_system *)void_ts;
	int i;
	double qhat;
	double score;
	int numBestTypes = 0;
	double bestScore;
	int bestTypes[ts->numTypes];
	if (multiplyWithAverage){
		C = C* averageValueOfAllTypes(ts);
	}
	// The multiplier is used to set the sign of the exploration bonus term (should be negative
	// for the min player and positive for the max player) i.e. so that we correctly compute
	// an upper confidence bound for Max and a lower confidence bound for Min
	double multiplier = (side == max) ? 1 : -1;
	int policyVisits;
	if (verbose){
		reportTypeStat(ts);
	}
	double trs[ts->numTypes];
	double sum_trs=0;
	for (i = 0; i < ts->numTypes; i++) {
		trs[i]= calculateTypeReachability(ts->types[i],bolzmanConstant);
		sum_trs+=trs[i];
	}
	for (i = 0; i < ts->numTypes; i++) {
		trs[i]= trs[i]/sum_trs;
	}
	
	for (i = 0; i < ts->numTypes; i++) { // iterate over all types
		//If the type has never been visited before, select it first
		if (ts->types[i]->visits == 0)
			return i;
		if (policy == MAB)
			policyVisits = ts->visits;
		else if (policy == KEEP_VMAB)
			policyVisits = ts->visits - ts->types[i]->birth + ts->types[i]->visits;
		else if (policy == DELETE_VMAB)
			policyVisits = ts->visits - ts->types[i]->birth;
		if (isUseMinmaxRewards){
			qhat = calcQhatBasedOnMinimax(ts,i);
			score = 1 * qhat + C * sqrt(log(policyVisits) / (double)ts->types[i]->visits); // add exploration component (-1 for minimization)
		}
		else {
			qhat = ts->types[i]->scoreSum / (double)ts->types[i]->visits;  // exploitation component (average utility or minimax value)
			if (verbose){
			   printf("C: %f qhat: %f level:%d \n ",C, qhat, countLevel(ts->types[i]));
			}
			score = qhat + (multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->visits); // add exploration component
			score = (1-probWeight) * score + 
				probWeight * trs[i] + 
				(multiplier * C) * sqrt(log(policyVisits) / (double)ts->types[i]->visits);
			// min(s1,s2,...) = -max(-s1,-s2,...) is min's turn means we can always just take the maximum
			score = (side == min) ? -score : score;
		}
		// If this is either the first child, or the best scoring child, store it
		if ((numBestTypes == 0) || (score > bestScore)) {
			bestTypes[0] = i;
			bestScore = score;
			numBestTypes = 1;
		}
		else if (score == bestScore) // if this child ties with the best scoring child, store it
			bestTypes[numBestTypes++] = i;
	}

	// Return the next type to explore (break ties randomly)
	return bestTypes[random() % numBestTypes];
}

//Generate the i'th child of a uct node
static void generateChild(treeNode *node, int i) {
	node->children[i] = calloc(1, sizeof(treeNode));
	node->children[i]->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
	node->children[i]->rep = _DOM->cloneRep(node->rep); // copy over the current board to child
	node->children[i]->side = node->side; // copy over the current side on move to child
	_DOM->makeMove(node->children[i]->rep, &(node->children[i]->side), i); //Make the i-th move
	node->children[i]->parent = node; //save parent
	node->children[i]->depth = node->depth + 1;
	node->children[i]->subtreeSize = 1;
}

double static mmfunction(int mmLevel,int level){
	if (level==0){
		return 1;
	}
	return (1- (mmLevel/level)); 

}
//TODO: change the way you handle empty open lists - should stop the main iteration loop and also maybe should not be returned from here or from select type
//An iteration of BFB:
//select the best type
//extract a node from its open list
//rollout and backpropagate from selected nodes
//place children in thier respective type open lists
static void bfbIteration(type_system *ts, double C, double CT, heuristics_t heuristic, int budget, int backupOp, int side,
	       	int threshold, int policy, double bolzmanConstant, double probWeight,treeNode* root, 
		int multiplyWithAverage, int isUseMinmaxRewards) { 
	int i;
	int typeId;
	if(root){
		typeId = selectTypeFromRoot(ts, CT, side, policy, backupOp, bolzmanConstant, probWeight, root);
	}else{
		typeId = selectType(ts, CT, side, policy, backupOp, bolzmanConstant, probWeight, multiplyWithAverage, isUseMinmaxRewards);
	}
	if (typeId == -1)  //Open lists are all empty
		return;
	type *t = ts->types[typeId];
	treeNode *node = ts->selectFromType(t, C);
	double ret;
	int gameOver = 0;
	if ((ret = _DOM->getGameStatus(node->rep)) != INCOMPLETE) {
		gameOver = 1;
		// This is a terminal node (i.e. can't generate any more children)
		// If we are estimating the leaf nodes using coarse random playout(s), coarsened h1 or random values, then all
		// those estimates are from the set {-1, 0, +1}. The terminal nodes are given values from the set {MIN_WINS, DRAW, MAX_WINS}
		// which are substantially larger. To make these values comparable in magnitude, we need to rescale the terminal
		// node values. If we are using engineered heuristics, then no rescaling is necessary.
		if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5))
			ret /= MAX_WINS; // rescale
	} else{
		ret = heuristic(node->rep, node->side, budget); 
	}
	//backpropagate path
	int mmlevelOfNode = minmaxLevel(node);
	treeNode *bp = node;
	int aboveTypeVTS = false;
	int aboveType = false;
	double bpMean;
	while (bp != NULL) {
		if (bp == ((type_sts*)t)->root){
			backupOp = MINMAX; // switch to minmax from type
		}
		bp->n++;
		if (backupOp == AVERAGE) {
			bp->scoreSum = bp->scoreSum + ret;
			if (!aboveType) {
				if (bp->n == 1 || (gameOver == 1 && bp == node)) 
					bp->minmax = ret;
				else { //compute minimax
					double bestScore = (bp->side == max) ? MIN_WINS : MAX_WINS;
					for (i = 1; i < _DOM->getNumOfChildren(); i++) {
						if (_DOM->isValidChild(bp->rep, bp->side, i)) { // if child exists, is it the best scoring child?
							if ((bp->side == max && bp->children[i]->minmax > bestScore) || (bp->side == min && bp->children[i]->minmax < bestScore))
								bestScore = bp->minmax;
						}
					}  
					bp->minmax = bestScore;
				}
				if (bp->typeDefiner == true)
					aboveType = true;
			}
			int nodeLevel = countLevelNode(bp);
			if (nodeLevel >=mmlevelOfNode){
				bp->minimaxScoreSum += mmfunction(mmlevelOfNode,nodeLevel);
				bp->minimax_n++;
			}
			bpMean = bp->scoreSum/bp->n;
			bp->sd =(bp->n>1)?((double)(bp->n - 2) / (double)(bp->n - 1)) * bp->sd + (ret - bpMean ) * (ret - bpMean) / ((double)bp->n):0;
			if (ts->name == VTS) {
				if (!aboveTypeVTS) {
					bp->typedN++;
					bp->typedScoreSum = bp->typedScoreSum + ret;
					bpMean = bp->typedScoreSum / (double)(bp->typedScoreSum);
					bp->sd = ((double)(bp->typedN - 2) / (double)(bp->typedN - 1)) * bp->sd + (ret - bpMean) * (ret - bpMean) / ((double)bp->typedN);
					if (bp == ((type_vts *)t)->root)
						aboveTypeVTS = true;
				}
			}
		} else if (backupOp == MINMAX) {
			//If first visit to node than the value is set by default      
			if (bp->n == 1) 
				bp->scoreSum = ret;
			else if (gameOver == 1 && bp == node)
				bp->scoreSum = bp->scoreSum + ret;
			else { //compute minimax
				double bestScore = (bp->side == max) ? MIN_WINS : MAX_WINS;
				for (i = 1; i < _DOM->getNumOfChildren(); i++) {
					if (_DOM->isValidChild(bp->rep, bp->side, i)) { // if child exists, is it the best scoring child?
						double score = bp->children[i]->scoreSum / (double)(bp->children[i]->n);
						if ((bp->side == max && score > bestScore) || (bp->side == min && score < bestScore))
							bestScore = score;
					}
				}
				bp->scoreSum = bestScore * bp->n;
			}
		}
		bp = bp->parent;
	}
	//update type stats
	t->visits++;
	if (backupOp == AVERAGE) {
		if(isUseMinmaxRewards){
			double mmLevelOfType = minmaxLevel(((type_sts*)t)->root);
			int typeLevel = countLevel(t);
			assert(mmLevelOfType <= countLevel(t));
			t->scoreSum = t->scoreSum + mmfunction(mmLevelOfType,typeLevel);
			assert(t->scoreSum/t->visits<=typeLevel); 
		}else{
			t->scoreSum = t->scoreSum + ret;
		}
	} else if (backupOp == MINMAX) {
		if (t->visits == 1)
			t->scoreSum = ret;
		else {
			double score = t->scoreSum / (double)(t->visits - 1);
			if ((side == max && ret > score) || (side == min && ret < score))
				t->scoreSum = ret * t->visits;
			else
				t->scoreSum = score * t->visits;
		}
	}
	if (gameOver) //terminal node - no need to expand
		return;
	//generate the children and place them in the type system
	int numOfChildren = 0; //number of children actually generated
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (!_DOM->isValidChild(node->rep, node->side, i)) // if the i^th move is illegal, skip it
			continue;
		generateChild(node, i);
		ts->assignToType(ts, node->children[i], typeId, threshold, policy);
		numOfChildren++;
	}
	//Update the size of the sub tree of each ancestor of the chosen node
	treeNode **path; //path from type root node to the chosen frontier node
	int pathLength=-1;
	if (ts->name == VTS) {
		t = ts->types[typeId]; //type might have split so we need to update the pointer
		pathLength = node->depth - ((type_vts *)t)->root->depth;
		path = calloc(pathLength, sizeof(treeNode *));
	}
	//assert(pathLength!=-1); //THIS FAILS... It means uninitialized variable!!!! TODO  BUG!!
	if (pathLength == 0)
		aboveType = true;
	else
		aboveType = false;
	bp = node;
	i = 0;
	while (bp != NULL) {
		bp->subtreeSize = bp->subtreeSize + numOfChildren;
		if (ts->name == VTS) {
			if (!aboveType) {
				bp->typedSubtreeSize = bp->typedSubtreeSize + numOfChildren;
				path[i++] = bp;
				if (bp == path[pathLength - 1]) //no need to log path anymore
					aboveType = true;
			}
		}
		bp = bp->parent;
	}
	if (ts->name == VTS) {
		if (pathLength > 0)
			typeSignificance(ts, (type_vts *)t, path);
		free(path);
	}
}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
int makeBFBMove(rep_t rep, int *side, int tsId, int numIterations, double C, double CT, heuristics_t heuristic, int budget, int* bestMoves, int* numBestMoves, int backupOp, int threshold, int policy, double bolzmanConstant, double probWeight, int isSelectTypeFromNode, int multiplyWithAverage, int isUseMinmaxRewards) {
	int i;
	double val;
	int bestMove = NULL_MOVE;
	double bestScore;
	treeNode* rootNode;
	type_system **type_systems = calloc(_DOM->getNumOfChildren(), sizeof(type_system *)); //Alocate a type system for every possible child of the root
	*numBestMoves = 0; // reset size of set of best moves

	// Create the root node of the UCT tree; populate the board and side on move fields
	rootNode = calloc(1, sizeof(treeNode));
	rootNode->children = calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
	rootNode->rep = _DOM->cloneRep(rep);
	rootNode->side = *side;
	rootNode->depth = 0;
	rootNode->subtreeSize = 1;
	//Generate the children of the root node
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) // if the i^th move is illegal, skip it
			continue;
		generateChild(rootNode, i);
		rootNode->subtreeSize++;
		//Allocate aa type system for the i'th child and assign him to it
		type_systems[i] = init_type_system(tsId, rep, *side);
		type_systems[i]->assignToType(type_systems[i], rootNode->children[i], -1, threshold, policy);
	}
	//Run specified number of BFB iterations
	//Starts at one because i is also used as the total number of visits
	for (i = 1; i < numIterations + 1; i++) {
		int move = selectMove(rootNode, C, false);
		type_system *ts = type_systems[move];
		ts->visits++;
		treeNode* nodeForIteration = NULL;
		if (isSelectTypeFromNode){
			nodeForIteration = rootNode->children[move];
		}
		bfbIteration(ts, C, CT, heuristic, budget, backupOp, *side, threshold, policy, bolzmanConstant, probWeight, nodeForIteration, multiplyWithAverage, isUseMinmaxRewards);
	}

	//Now look at the children 1-ply deep and determing the best one (break ties randomly)
	for (i = 1; i < _DOM->getNumOfChildren(); i++) { //For each move
		if (!_DOM->isValidChild(rootNode->rep, rootNode->side, i)) //Skip illegal moves
			continue;

		if (!rootNode->children[i]) //This node was not created since # iterations was too small
			continue;

		//Compute average utility of this child
		val = rootNode->children[i]->scoreSum / (double)rootNode->children[i]->n;
		//If this was min's move, negate the utility value (this makes things a little cleaner
		// as we can then always take the max of the children, since min(s1,s2,...) = -max(-s1,-s2,...))
		val = (*side == min) ? -val : val;

		//If this is the first child, or the best scoring child, then store it
		if ((*numBestMoves == 0) || (val > bestScore)) {
			bestMoves[0] = i;
			bestScore = val;
			*numBestMoves = 1;
		}
		else if (val == bestScore) //Child ties with currently best scoring one; store it
			bestMoves[(*numBestMoves)++] = i;

		if (verbose)
			printf("Move # %d -- Value %f, Count %d\n", i, ((*side == min) ? -val : val), rootNode->children[i]->n);
	}

	//We should have at least looked at one child
	assert(*numBestMoves != 0);

	bestMove = bestMoves[random() % *numBestMoves]; // pick the best move (break ties randomly)
	//bestMove = bestMoves[0];
	_DOM->makeMove(rep, side, bestMove); // make it (updates game state)

	if (verbose) {
		printf("Value of root node: %f\n", rootNode->scoreSum / (double)rootNode->n);
		printf("Best move: %d\n", bestMove);
	}

	// Clean up before returning
	freeTree(rootNode);
	freeTypeSystems(type_systems);

	return bestMove;
}
