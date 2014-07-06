#include "common.h"
#include "domain.h"
#include "uct.h"
#include "phi.c"
#include "t_values.c"
#include "wilcoxon.c"

double standardDeviation(treeNode *node) {
  return sqrt(node->M2 / (double)(node->n - 1));
}

double confidenceInterval(treeNode *node) {
  double variance = node->M2 / (double)(node->n - 1);
  double criticalValue;
  
  if (node->n < 1500)
    criticalValue = t_values[node->n - 1];
  else
    criticalValue = z975;
  
  return 2 * criticalValue * sqrt(variance / (double)node->n);
}

void subset_backup(treeNode *node, double ret, int ci_threshold, double (confidenceMeasure)(treeNode *)) {
		updateStatistics(node, ret);
		
		if (node->n < ci_threshold)
		  return;

		int i;
		double score, bestScore = avgRewards(node);//Average score of the parent because we only consider children that are better than that
		bestScore = node->side == max ? bestScore : -bestScore;
		double bestChildConfidence = INF;
		node->ci = confidenceMeasure(node);
		
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i] && node->children[i]->n >= ci_threshold) { // if child exists, is it the best scoring child?
				score = assignedScore(node->children[i]);
				score = node->side == max ? score : -score;
				
				if (node->children[i]->ci < node->ci && score > bestScore) {
					bestScore = score;
					bestChildConfidence = node->children[i]->ci;
				}
			}
		}
		
		if (bestChildConfidence < INF) {
			bestScore = node->side == max ? bestScore : -bestScore;//reversing
			node->scoreSum = node->n * bestScore;
			node->ci = bestChildConfidence;
			return;
		}

		node->scoreSum = node->realScoreSum; //reset score to average of current node
}


void size_backup(treeNode *node, double ret, int ci_threshold) {
		updateStatistics(node, ret);
		
		if (node->n < ci_threshold)
		  return;
		
		int i, bestChild = 0;
		double score, bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
	
		for (i = 1; i < _DOM->getNumOfChildren(); i++) {
			if (node->children[i] && node->children[i]->n >= ci_threshold) { // if child exists, is it the best scoring child?
				score = assignedScore(node->children[i]);
				if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore))) {
					bestScore = score;
					bestChild = i;
				}
			}
		}

		if (bestChild > 0) {
			node->scoreSum = node->n * bestScore;
			return;
		}
		
		node->scoreSum = node->realScoreSum; // reset score to average of current node
}



/**
 *
 * Update function as described by coulom 2007 paper.
 *
 * */
void coulom(treeNode *node, double ret){
  int i, maxVisits=-1, childWithBestScore, childWithMostVisits;
  double bestScore, score;
  bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
  updateStatistics(node,ret);
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (node->children[i]) { 
	    if (node->children[i]->n > maxVisits){
		    maxVisits = node->children[i]->n; 
		    childWithMostVisits = i;
	    } 
	    score = assignedScore(node->children[i]);
	    if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore))){
		    bestScore = score;
		    childWithBestScore = i;
	    }
    }
  }
  if(childWithBestScore == childWithMostVisits){
	  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
  } 
}



void minmax_backup(treeNode *node) {
  int i;
  double bestScore, score;
	
  (node->n)++;
  bestScore = (node->side == max) ? MIN_WINS : MAX_WINS;
  
  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
    if (node->children[i]) { // if child exists, is it the best scoring child?
      score = assignedScore(node->children[i]);
      if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore)))
	bestScore = score;
    }
  }
  
  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
}


/**
 * A non-parameteric approach
 */
void wilcoxon_backup(treeNode *node, double ret){
	int i,numOfVectors=0,winner=-1;
	updateStatistics(node,ret); //updates the average
	vector* childData[_DOM->getNumOfChildren()];
	childData[0] = NULL;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	    if (node->children[i]) { 
		    childData[i] = allLeaves(node->children[i]);
		    numOfVectors++;
	    }else{
		    childData[i] = NULL;
	    }
	}
	if (numOfVectors>1){
		winner = wilcoxon_winner(childData,_DOM->getNumOfChildren()); 
	}
	//assert(winner==-1);
	if (winner != -1){
		node->scoreSum = (node->n) * (node->children[winner]->scoreSum/node->children[winner]->n);
	} 
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (childData[i]){
			destroyVector(childData[i]);
			free(childData[i]);
		}
	}
}

void variance_backup(treeNode *node, double ret, int ci_threshold) {
		if (node->n < ci_threshold) {
			updateStatistics(node, ret);
			return;
		}

		int i;
		double bestScore, score;
		
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
		
		//(*parentCITotal)++;
		if (bestSD < node->ci) {
		  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
		  node->ci = bestSD;
		  updateStatistics(node, ret);
		  node->scoreSum -= ret; //updateStatistics also adds the reward to the scoreSum which shouldn't happen in this case.
		  
		  //(*parentCIWin)++;
		  //*avgDepth = ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
		  //if(*minDepth > node->depth)
		  //	*minDepth = node->depth;
		  
		} else {
		  node->scoreSum = node->realScoreSum; // reset score to average of current node
		  updateStatistics(node, ret);
		}
}

void ci_backup(treeNode *node, double ret, int ci_threshold) {
		int i;
		double bestScore, score;
  
		if (node->n < ci_threshold) {
		  updateStatistics(node, ret);
		  return;
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
		
		//(*parentCITotal)++;
		if (bestCI < node->ci) {
		  node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
		  node->ci = bestCI;
		  updateStatistics(node, ret);
		  node->scoreSum -= ret; //updateStatistics also adds the reward to the scoreSum which shouldn't happen in this case.
		  
		  //(*parentCIWin)++;
		  //*avgDepth = ((*avgDepth * (*parentCIWin-1)) + node->depth) /(*parentCIWin);
		  //if(*minDepth > node->depth)
		//	*minDepth = node->depth;
		  
		} else {
		  node->scoreSum = node->realScoreSum; // reset score to average of current node
		  updateStatistics(node, ret);
		}
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

void weighted_mm_backup(treeNode *node, double ret) {
    updateStatistics(node,ret);
    double ttlPp=0;
    double nodeScore=0;
    double nodeP,nodeVal;
    int i;

    for (i = 1; i < _DOM->getNumOfChildren(); i++) {
      if (node->children[i]) { // if child exists, is it the best scoring child?
	nodeP=getProbForMaximialChild(i,node);
	ttlPp+=nodeP;
	nodeVal = node->children[i]->scoreSum / (double)node->children[i]->n;
	nodeScore+=(nodeVal*nodeP);
      }
    }
    node->scoreSum = (node->n) * (nodeScore/ttlPp); 
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
