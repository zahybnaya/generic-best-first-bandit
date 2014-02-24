#include "type_reachability.h"
/***
 * Returns the pseudo-probablistic value of a given difference
 * Boltzman distribution
 *
 * */ 
 double probValue(double difference, double bestSd, double mySd,double bolzmanConstant){
	 if(difference > 0 || fabs(difference) <0.0001){
		//printf(" =>%f\n",1.0);
	 	return 1;
	 }
	//double bolzmanConstant = -20.4; 
	double base = 2.71828;
	double deltaT = sqrt(2*(pow(mySd,2)+pow(bestSd,2)));
	double c = pow(base,bolzmanConstant *(-difference/deltaT));
	//printf(" =>%f\n",c);
	return c;
}


/**
 * Returns the minimal difference of node n over its siblings
 * negative values means that this this type is not good 
 *
 * Value(n)-Value(n*)
 */
bolzman_args minDiff(treeNode* n){
	treeNode* parent = n->parent; 
	int side = n->side;
	double best = (side == max)? -INF : INF;
	double bestSd;
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		treeNode* child = parent->children[i];
		if (!child) 
			continue;
		double childValue = child->scoreSum/child->n; 
		if ((side==max &&  childValue > best) || 
				(side==min && childValue < best)) {
			best = childValue;
			bestSd = child->sd;
		}
	}
	double nvalue = n->scoreSum/n->n; 
	double delta  =  (nvalue - best) * ((side == max)?1:-1);
	bolzman_args args = {delta, bestSd, n->sd};
	//printf("%f,",delta);
	return args;
}


/**
 * Calculate a pseudo-probability coefficient for type t 
 * assumes that t is type_sts 
 */
 double calculateTypeReachability(type* t,double bolzmanConstant  ){
	double c =1.0;
	treeNode* n = ((type_sts*)t)->root;
	while(n->parent != NULL){
		bolzman_args args =minDiff(n);
		c*= probValue(args.difference, args.bestSd, args.mySd,bolzmanConstant);
		n = n->parent;
	}
	return c;
}

/**
 * Level of node n (0 is the root)
 * */
 int countLevelNode(treeNode* n){
	int level = 0;
	while(n->parent != NULL){
		level++; 
		n = n->parent;
	}
	return level;

}

/**
 * Level of type t (0 is the root)
 * */
 int countLevel(type* t){
	return countLevelNode(((type_sts*)t)->root);
}


/**
 *
 * Calculate whether this type is a maximum or a minimum 
 */
 int calculateSideOfType(type* t){
	treeNode* n = ((type_sts*)t)->root;
	return 	n->side;
}

/**
 * What is the level untill which node n is a "priciple node" 
 * (root is zero)
 */
 double minmaxLevel(treeNode* n){
	while(n->parent != NULL){
		if(minDiff(n).difference<0){
			break;
		}
		n = n->parent;
	}
	return countLevelNode(n);
}
/***
 * Average value for all types (minimax reward)
 * */ 
 double averageValueOfAllTypes(type_system* ts) 
{
	int i;
	double r = 0;
	int n=0;
	for (i = 0; i < ts->numTypes; i++) {
		if(((type_sts*)ts->types[i])->visits>0){
			r+=(ts->types[i]->scoreSum/((type_sts*)ts->types[i])->visits);
			n++;
		}
	}
	return n>0?(r/n):1.0;
}

/**
 *  Calcs exploitation term based on the mm rewards 
 * */ 
 double calcQhatBasedOnMinimax(type_system *ts, int i)
{
	return ts->types[i]->scoreSum / (double)((type_sts*)ts->types[i])->visits;  // exploitation component (this is the average utility or minimax value)
}



