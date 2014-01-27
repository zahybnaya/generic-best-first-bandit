#include "common.h"
#include "type.h"


 double probValue(double difference);
 double minDiff(treeNode* n);
 double calculateTypeReachability(type* t);
 int countLevelNode(treeNode* n);
 int countLevel(type* t);
 int calculateSideOfType(type* t);
 double minmaxLevel(treeNode* n);
 double averageValueOfAllTypes(type_system* ts); 
 double calcQhatBasedOnMinimax(type_system *ts, int i);
