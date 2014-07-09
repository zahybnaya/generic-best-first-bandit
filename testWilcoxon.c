#include <stdio.h>
#include "domain.h"
#include "common.h"
#include "uct.h"
#include "wilcoxon.c"

DOM* _DOM;
void testVector()
{
	vector v = createVector(0);
	printf("size is %d and capcaity %d \n", v.size, v.capacity);
	pushVector(&v,1.2);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.6);
	printf("size is %d and capcaity %d\n", v.size, v.capacity);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.2);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.3);
	pushVector(&v,1.4);
	pushVector(&v,1.5);
	pushVector(&v,1.6);
	printf("size is %d and capcaity %d\n", v.size, v.capacity);
	destroyVector(&v);
}
/*
 *  Creates the root of the tree
 * */
static treeNode* createRootNode() {
	// Create the root node of the UCT tree; populate the board and side on move fields
	treeNode* rootNode;
	rootNode = (treeNode*)calloc(1, sizeof(treeNode));
	rootNode->children = (treeNode**)calloc(_DOM->getNumOfChildren(), sizeof(treeNode*));
	return rootNode;
}

treeNode* buildTree(){
	treeNode* root = createRootNode();
	root->n=6;
	root->children[1]=createRootNode();
	root->children[1]->scoreSum = 10;
	root->children[1]->n = 3;
	root->children[1]->children[1] = createRootNode();
	root->children[1]->children[1]->scoreSum = 6;
	root->children[1]->children[2] = createRootNode();
	root->children[1]->children[2]->scoreSum = 5;
	root->children[2]=createRootNode();
	root->children[2]->scoreSum = 11;
	root->children[3]=createRootNode();
	root->children[3]->scoreSum = 12;
	return root;
}

void test_allLeaves(){
	vector v =allLeaves(buildTree()); 
	printf("size is %d and capcaity %d\n", v.size, v.capacity);
	printf("elemnt is %f \n", v.array[0]);
	printf("elemnt is %f \n", v.array[1]);
	printf("elemnt is %f \n", v.array[2]);
	printf("elemnt is %f \n", v.array[3]);
}


int f()
{
	return 5;
}

void test_CountItems(){
	double arr[] = { 480, 480, 281, 310, 390, 308, 308, 308, 291, 302, 371, 406, 403,
		275, 272, 291, 344, 293, 287, 337, 314, 344, 330, 312};
	double arr2[] = {920, 807, 664, 871, 1072, 646 ,503 ,697, 563 ,574 ,651 ,763 ,574 ,678 ,732 ,551 ,438,
	441, 448, 611, 633, 707, 453, 582};
	int n1= sizeof(arr)/sizeof(double);
	int n2 = sizeof(arr2)/sizeof(double);
	int i,item_c=n1+n2,counter=0;
	printf("%d\n",item_c);
	item* items[item_c];
	item actualItems[item_c];
	for (i = 0; i < n1; i++) {
		item it;
		it.v=arr[i];
		it.set=1;
		actualItems[counter]=it;
		items[counter]=&actualItems[counter];
		counter++;
	}
	for (i = 0; i < n2; i++) {
		item it;
		it.v=arr[i];
		it.set=2;
		actualItems[counter]=it;
		items[counter]=&actualItems[counter];
		counter++;
	}
	addRanks(items,item_c);
	print_items(items,item_c);
	double set1, set2;
	countItems(items, item_c, &set1, &set2);
	printf("should be 15:%f\n",set1);
	printf("should be 10:%f\n",set2);
}
void test_wilcoxon_winner(){
	vector v1,v2,v3,v4,v5;
	double arr[] = { 11480, 480319, 480281, 1110, 3190, 348008, 3116, 3125, 1291, 3102, 1371,14106, 4103,
		2751, 2721, 2911, 3441, 2193,111287, 31137,4180314, 3144, 4180330, 3112,11480, 480319, 480281, 1110, 3190, 348008, 3116, 3125, 1291, 3102, 1371,14106, 4103,
		2751, 2721, 2911, 3441, 2193,111287, 31137,4180314, 3144, 4180330, 3112};
	v1.array=arr;
	v1.size = sizeof(arr)/sizeof(double);
	v1.capacity = sizeof(arr)/sizeof(double);

	double arr2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	v2.array=arr2;
	v2.size = sizeof(arr2)/sizeof(double);
	v2.capacity = sizeof(arr2)/sizeof(double);

	double arr3[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	v3.array=arr3;
	v3.size = sizeof(arr3)/sizeof(double);
	v3.capacity = sizeof(arr3)/sizeof(double);

	double arr4[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	v4.array=arr4;
	v4.size = sizeof(arr4)/sizeof(double);
	v4.capacity = sizeof(arr4)/sizeof(double);

	double arr5[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	v5.array=arr5;
	v5.size = sizeof(arr5)/sizeof(double);
	v5.capacity = sizeof(arr5)/sizeof(double);
	vector childData[5] = {v2,v3,v1,v4,v5};
	int res=wilcoxon_winner(childData,5);	
	printf("who should win? %d\n",res);
}
void test_wilcoxon_winner_fo_two(){
	vector v1,v2;
	double arr[] = { 11480, 480319, 480281, 1110, 3190, 348008, 3116, 3125, 1291, 3102, 1371,14106, 4103,
		2751, 2721, 2911, 3441, 2193,111287, 31137,4180314, 3144, 4180330, 3112,11480, 480319, 480281, 1110, 3190, 348008, 3116, 3125, 1291, 3102, 1371,14106, 4103,
		2751, 2721, 2911, 3441, 2193,111287, 31137,4180314, 3144, 4180330, 3112};
	v1.array=arr;
	v1.size = sizeof(arr)/sizeof(double);
	v1.capacity = sizeof(arr)/sizeof(double);
	double arr2[] = {920, 807, 664, 871, 1072, 646 ,503 ,697, 563 ,574 ,651 ,763 ,574 ,678 ,732 ,551 ,438,
	441, 448, 611, 633, 707, 453, 582};
	v2.array=arr2;
	v2.size = sizeof(arr2)/sizeof(double);
	v2.capacity = sizeof(arr2)/sizeof(double);
	int res=wilcoxon_winner_of_two(&v2,&v1);	
	printf("who should win? %d\n",res);
	int res2=wilcoxon_winner_of_two(&v1,&v2);	
	printf("who should win? %d\n",res2);
}
//
//void test_wilcoxon_winner_fo_two(){
//	vector v1,v2;
//	double arr[] = { 480, 319, 281, 310, 390, 308, 316, 325, 291, 302, 371, 406, 403,
//		275, 272, 291, 344, 293, 287, 337, 314, 344, 330, 312};
//	v1.array=arr;
//	v1.size = sizeof(arr)/sizeof(double);
//	v1.capacity = sizeof(arr)/sizeof(double);
//	double arr2[] = {920, 807, 664, 871, 1072, 646 ,503 ,697, 563 ,574 ,651 ,763 ,574 ,678 ,732 ,551 ,438,
//	441, 448, 611, 633, 707, 453, 582};
//	v2.array=arr2;
//	v2.size = sizeof(arr2)/sizeof(double);
//	v2.capacity = sizeof(arr2)/sizeof(double);
//	int res=wilcoxon_winner_of_two(&v2,&v1);	
//	printf("who should win? %d\n",res);
//	int res2=wilcoxon_winner_of_two(&v1,&v2);	
//	printf("who should win? %d\n",res2);
//}
//
void testAddRanks(){
	double arr[] = { 480, 480, 281, 310, 390, 308, 308, 308, 291, 302, 371, 406, 403,
		275, 272, 291, 344, 293, 287, 337, 314, 344, 330, 312};
	double arr2[] = {920, 807, 664, 871, 1072, 646 ,503 ,697, 563 ,574 ,651 ,763 ,574 ,678 ,732 ,551 ,438,
	441, 448, 611, 633, 707, 453, 582};
	int n1= sizeof(arr)/sizeof(double);
	int n2 = sizeof(arr2)/sizeof(double);
	int i,item_c=n1+n2,counter=0;
	printf("%d\n",item_c);
	item* items[item_c];
	item actualItems[item_c];
	for (i = 0; i < n1; i++) {
		item it;
		it.v=arr[i];
		it.set=1;
		actualItems[counter]=it;
		items[counter]=&actualItems[counter];
		counter++;
	}
	for (i = 0; i < n2; i++) {
		item it;
		it.v=arr[i];
		it.set=2;
		actualItems[counter]=it;
		items[counter]=&actualItems[counter];
		counter++;
	}
	print_items(items,item_c);
	addRanks(items,item_c);
	print_items(items,item_c);
}

int main(int argc, const char *argv[])
{
	_DOM = (DOM*)malloc(sizeof(DOM));
	_DOM->getNumOfChildren = f;
	test_wilcoxon_winner();
	return 0;
}
