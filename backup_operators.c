#include "common.h"
#include "domain.h"
#include "uct.h"
#include "phi.c"
#include <math.h>
#include "t_values.c"
#include "wilcoxon.c"

typedef int(*inferrior_criterion)(treeNode* n1, treeNode* n2, double(sd_calc)(treeNode*),int side);

static double supperrior_sum(treeNode* node, BOOL inferriors[], double* outAggScoreSum,int* outAggVisits );


/**
 * Returns the combined variance of all non-inferrior children
 * */
double standardDeviationAggregated(treeNode *node,BOOL inferriors[]) {
	int i, all_visits=0;
	double var=0.0,mu,mux=0;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (!inferriors[i] && node->children[i]){
			all_visits+=node->children[i]->n;
			mu=node->children[i]->scoreSum/node->children[i]->n;
			mux+=node->children[i]->scoreSum;
			var+=(node->children[i]->n-1)*node->children[i]->ci + node->children[i]->n*pow(mu,2);
			//printf("var:%f (node->children[i]->n-1):%d,node->children[i]->ci:%f,  node->children[i]->n: %d pow(mu,2):%f\n",var,(node->children[i]->n-1),node->children[i]->ci , node->children[i]->n,pow(mu,2));
		}
	}
	var -= pow(mux,2)/all_visits;
	//printf("var:%f -=all_visits:%d*pow(node->scoreSum/node->n,2):%f",var,all_visits,pow(node->scoreSum/node->n,2));
	if (all_visits>1){
		var = (1/(double)(all_visits-1))*var;
	}
	//printf("SD: Usuall value would be %f. But we return %f\n",sqrt(node->M2 / (double)(node->n - 1)), var);
	return var;
}

double standardDeviation(treeNode *node) {
  return sqrt(node->M2 / (double)(node->n - 1));
}



int satterthaiteWelch(double s1, double s2, int n1, int n2){
	if (n1==1 || n2==1){
		return 1;
	}
	double df = pow(s1/n1 + s2/n2,2);
	if (df>0) {
		double denom = pow(s1/n1,2)/(n1-1) +  pow(s2/n2,2)/(n2-1); 
		if (denom > 0){
		//	printf("%f:%f=%d   s1:%f,s2:%f,n1:%d,n2:%d  \n",df,denom,(int)(df/denom),s1,s2,n1,n2);
			return df/denom; 
		}
	}
	return 1;
}


int t_test(treeNode* n1, treeNode* n2, double(sd_calc)(treeNode*),int side){
	double x1=n1->scoreSum/n1->n;
	double x2=n2->scoreSum/n2->n;
	double s1=sd_calc(n1);
	double s2=sd_calc(n2);
	double t = (x1-x2)/sqrt(s1/n1->n+s2/n2->n);
	int k =  satterthaiteWelch(s1,s2,n1->n,n2->n);
	//printf("size of t values: %d sw result: %d",(int)(sizeof(t_values)/sizeof(double)-1),k);
	k = k>(int)(sizeof(t_values)/sizeof(double)-1)?(int)(sizeof(t_values)/sizeof(double)-1):k;
	//printf("k ending up being: %d\n",k-1);
	double t_critical = t_values[k - 1];
	if (abs(t)<t_critical){
		return 0;
	}
	if (side==min) {
		t*=-1;
	}
	return t>0?1:-1;
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


double getCi(treeNode* n){return n->ci;};


int variance_test(treeNode* n1,treeNode* n2,double(sd_calc)(treeNode*),int side){
  if (sd_calc(n1) < sd_calc(n2)){
	  double n1_val = n1->scoreSum/n1->n;
	  double n2_val = n2->scoreSum/n2->n;
	  if ((side == max)? (n1_val > n2_val) :  (n2_val > n1_val)){
		  return 1;
	  }
  }
  return 0;
}

/**
 * Fills the boolean of which child is inferrior according to t test
 * */
void detect_inferriors(treeNode* node,BOOL inferriors[], inferrior_criterion criterion )
{
	int i,j,infer_i;
	double t;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (!node->children[i]){
			continue;
		} 
		for (j = i; j < _DOM->getNumOfChildren(node->rep, node->side); j++) {
			if (!node->children[j]){
				continue;
			}
			t = criterion(node->children[i],node->children[j],getCi,node->side);
			if (t==0){
				continue;
			}
			infer_i=t>0?j:i; // positive means i > j
			int other = infer_i==i?j:i;
			assert(node->side==max?((node->children[infer_i]->scoreSum/node->children[infer_i]->n) <
				       (node->children[other]->scoreSum/node->children[other]->n)):
				       ((node->children[infer_i]->scoreSum/node->children[infer_i]->n) >
				       (node->children[other]->scoreSum/node->children[other]->n))
				       );
			inferriors[infer_i]=true;
		}
	}
}

/**
 *  Returns the cdp value of the tree
 *  Recursively computes the cdp value of the UCT-constructed tree rooted at 'node' 
 **/
double cdp_tree_value(treeNode* node,inferrior_criterion criterion){
	int i;
	double val;
	assert(node != NULL);
	if ((val = _DOM->getGameStatus(node->rep)) != _DOM->incomplete){
		node->ci = 0;
		node->scoreSum = val/_DOM->max_wins;//rescale
		return node->scoreSum;
	}
	if (node->n == 1) {
	  assert((node->scoreSum > _DOM->min_wins) && (node->scoreSum < _DOM->max_wins)); // make sure heuristic is bounded by terminal node values
	  node->ci=0;
	  return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
	}
	int count_children = _DOM->getNumOfChildren(node->rep, node->side);
	BOOL* inferriors = (BOOL*)calloc(count_children,sizeof(BOOL));
	for (i = 1; i < count_children; i++) {
		treeNode* child=node->children[i];
		if (child) { // only descend if child exists
			val = cdp_tree_value(child,criterion); 
		}
	}
	detect_inferriors(node,inferriors,criterion);
	node->ci = standardDeviationAggregated(node,inferriors);
	//printf("setting node->ci to be %f\n",node->ci);
	double sum = supperrior_sum(node,inferriors,&node->scoreSum,&node->n);
	free(inferriors);
	return sum;
}



void print_inferroris(BOOL inferriors[], int count)
{
	printf("INFER:[");
	int i;
	for (i = 0; i < count; i++) {
		printf("%d", inferriors[i]);
	}
	printf("]\n");

}

static double calculateFirstReward(treeNode* node)
{
	double aggScoreSum=0;
	int aggVisits=0,i;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (node->children[i]) { 
			aggScoreSum+=node->children[i]->realScoreSum;
			aggVisits+=node->children[i]->n;
		}
	}
	//assert(node->n-aggVisits==1);
	return (node->realScoreSum - aggScoreSum);
}

static double supperrior_sum(treeNode* node, BOOL inferriors[], double* outAggScoreSum,int* outAggVisits )
{
	double firstReward=calculateFirstReward(node);
	double aggScoreSum=0;
	int aggVisits=0,i;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		if (!inferriors[i] &&  node->children[i]) { 
			aggScoreSum+=node->children[i]->scoreSum;
			aggVisits+=node->children[i]->n;
		}
	}
	*outAggVisits = aggVisits+1;
	*outAggScoreSum = aggScoreSum+firstReward;
	return (*outAggScoreSum/ *outAggVisits ) ;
}


void inferrior_nodes(treeNode *node, double ret ) {
	updateStatistics(node, ret);
	int i;
	BOOL inferriors[_DOM->getNumOfChildren(node->rep, node->side)];
	for (i = 0; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
		inferriors[i]=false;
	}
	detect_inferriors(node,inferriors,t_test);
	double* dummy1=0; 
	int* dummy2=0;
	int ss = supperrior_sum(node,inferriors,dummy1,dummy2);  
	node->scoreSum =  node->n*ss;
}

void subset_backup_agg(treeNode *node, double ret, int ci_threshold, double (confidenceMeasure)(treeNode *)) {
		updateStatistics(node, ret);
		if (node->n < ci_threshold)
		  return;
		int i;
		double score, bestScore = avgRewards(node);//Average score of the parent because we only consider children that are better than that
		bestScore = node->side == max ? bestScore : -bestScore;
		double bestChildConfidence = INF;
		node->ci = confidenceMeasure(node);
		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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


		double aggScoreSum=0;
		int aggVisits=0;
		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
			if (node->children[i]) { 
				aggScoreSum+=node->children[i]->scoreSum;
				aggVisits+=node->children[i]->n;
			}
		}
		node->scoreSum =  node->n*(aggScoreSum/aggVisits); //aggregated value
		//printf("Usuall value would be %f. But we return %f\n",node->realScoreSum,node->scoreSum);
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
		
		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
		double score, bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;
	
		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
void coulom(treeNode *node, double ret) {
  int i, maxVisits=-1, childWithBestScore, childWithMostVisits;
  double bestScore, score;
  bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;
  updateStatistics(node,ret);
  for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
  bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;
  
  for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
	vector* childData[_DOM->getNumOfChildren(node->rep, node->side)];
	childData[0] = NULL;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
	    if (node->children[i]) { 
		    //childData[i] = allLeaves(node->children[i]);
		    numOfVectors++;
	    }else{
		    childData[i] = NULL;
	    }
	}
	if (numOfVectors>1){
		winner = wilcoxon_winner(childData,_DOM->getNumOfChildren(node->rep, node->side)); 
	}
	//assert(winner==-1);
	if (winner != -1){
		node->scoreSum = (node->n) * (node->children[winner]->scoreSum/node->children[winner]->n);
	} 
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
		
		bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;
		double bestSD = INF;

		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
		
		bestScore = (node->side == max) ? _DOM->min_wins : _DOM->max_wins;
		double bestCI = INF;
		
		for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
	if ((xi == _DOM->min_wins && node->side == min) || (xi == _DOM->max_wins && node->side == max) ){
		return 1;
	}
	if ((xj == _DOM->min_wins && node->side == min) || (xj == _DOM->max_wins && node->side == max) ){
		return 0;
	}
	double sdi=0.5,sdj=0.5;
	if(node->children[i]->n >1)
		sdi = node->children[i]->M2/(double)(node->children[i]->n - 1);
	if(node->children[j]->n >1)
		sdj = node->children[j]->M2/(double)(node->children[j]->n - 1);
	double s = pow((sdi+sdj),1);
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
	double pr, logAnswer=0;
	treeNode* child = node->children[mi];
	if (!child)
		return 0;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
	  if (i==mi||!node->children[i]) { 
		  continue;
	  }
	  pr= getProbForGreater(mi,i,node);
	  logAnswer+=log(pr);
	}
	printf("%f\n",exp(logAnswer));
	return exp(logAnswer);
}

/***
 * An aggregation that sums ,for each child i, Pr(i==max)*Val(i) 
 */
void weighted_mm_backup(treeNode *node, double ret) {
    updateStatistics(node,ret);
    double ttlPp=0, nodeScore=0, nodeP,nodeVal;
    int i;

    for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
      if (node->children[i]) { 
	nodeP=getProbForMaximialChild(i,node);
	ttlPp+=nodeP;
	nodeVal = node->children[i]->scoreSum / (double)node->children[i]->n;
	nodeScore+=(nodeVal*nodeP);
      }
    }
    node->scoreSum = (node->n) * (nodeScore/ttlPp); 
}

/* Weighted Minimax for static tree*/
static double weightedMM(treeNode* node,heuristics_t heuristic) {
	int i;
	double val;
	double bestScore = 0;
	char s[1512],b[256]; 
	sprintf(s,"*side:%d visits:%d depth:%d ",node->side,node->n,node->depth);
	if ((val = _DOM->getGameStatus(node->rep)) != _DOM->incomplete){
		if ((heuristic == _DOM->hFunctions.h3) || (heuristic == _DOM->hFunctions.h4) || (heuristic == _DOM->hFunctions.h5)) {
			val = val / _DOM->max_wins;
		}
		return val; // return something from the set {MIN_WINS, DRAW, MAX_WINS}
	}
	if (node->n == 1) {
	  assert((node->scoreSum > _DOM->min_wins) && (node->scoreSum < _DOM->max_wins)); 
	  return node->scoreSum; // the node was already evaluated when doing UCT, so just use that value
	}
	double ttlPp=0;
	double rttlp=0;
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
	  if (node->children[i]) { // only descend if child exists
	    ttlPp+=getProbForMaximialChild(i,node); 
	  }
	}
	assert(ttlPp>0);
	for (i = 1; i < _DOM->getNumOfChildren(node->rep, node->side); i++) {
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
	return bestScore;
}
