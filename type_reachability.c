#include "type_reachability.h"
/***
 * Returns the pseudo-probablistic value of a given difference
 *
 * */ 
 double probValue(double difference){
	double base = 1.01; 
	double xp = 120;
	double constanct=0.3;  
	double c = pow(base,(xp*difference))-constanct;
	if (c > 1){
		return 1.0;
	}
	if (c < 0){
		return 0.0;
	}
	return c;

}


/**
 * Returns the minimal difference of node n over its siblings
 * negative values means that this this type is not good 
 *
 * Value(n)-Value(n*)
 */
 double minDiff(treeNode* n){
	treeNode* parent = n->parent; 
	int side = n->side;
	double best = (side == max)? -INF : INF;
	int i;
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		treeNode* child = parent->children[i];
		if (!child) 
			continue;
		double childValue = child->scoreSum/child->n; 
		if ((side==max &&  childValue > best) || 
				(side==min && childValue < best)) {
			best = childValue;
		}
	}

	double nvalue = n->scoreSum/n->n; 
	double delta  =  (nvalue - best) * ((side == max)?1:-1);
	return delta;
}


/**
 * Calculate a pseudo-probability coefficient for type t 
 * assumes that t is type_sts 
 */
 double calculateTypeReachability(type* t){
	double c =1.0;
	treeNode* n = ((type_sts*)t)->root;
	while(n->parent != NULL){
		if(minDiff(n)>0){
			c*= probValue(minDiff(n));
		}
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
		if(minDiff(n)<0){
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
	for (i = 0; i < ts->numTypes; i++) {
		r+=(ts->types[i]->scoreSum/((type_sts*)ts->types[i])->mm_visits);
	}
	return r/ts->numTypes;

}

/**
 *  Calcs exploitation term based on the mm rewards 
 * */ 
 double calcQhatBasedOnMinimax(type_system *ts, int i)
{
	return ts->types[i]->scoreSum / (double)((type_sts*)ts->types[i])->mm_visits;  // exploitation component (this is the average utility or minimax value)
}



