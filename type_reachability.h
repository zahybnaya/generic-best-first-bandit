#include "common.h"
#include "type.h"

typedef struct bolzman_args_s {
	double difference;
	double bestSd;
	double mySd;
} bolzman_args;

 double probValue(double difference, double, double,double);
 bolzman_args minDiff(treeNode* n);
 double calculateTypeReachability(type* t,double);
 int countLevelNode(treeNode* n);
 int countLevel(type* t);
 int calculateSideOfType(type* t);
 double minmaxLevel(treeNode* n);
 double averageValueOfAllTypes(type_system* ts); 
 double calcQhatBasedOnMinimax(type_system *ts, int i);
