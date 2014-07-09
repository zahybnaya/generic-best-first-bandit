#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define  Z_UPPER 4.96
#define  Z_LOWER -4.96 

/*==================================================================== 
 *  Vector struct
 *====================================================================*/
typedef struct vector {
 	double* array;
	int capacity; 
	int size;
} vector;

typedef struct item{double v; int set; double rank;} item; 

vector* createVector(int capacity){
	vector* v = (vector*)malloc(sizeof(vector));
	v->capacity=capacity>10?capacity:10;
	v->array = (double*)malloc(sizeof(double)*v->capacity);
	v->size = 0;
	return v;
}

void pushVector(vector* v, double val){
	if (v->size >= v->capacity){
		int newCapacity=v->capacity*2;
		double* n_add = (double*)realloc(v->array, sizeof(double)*newCapacity);
		if (n_add) {
			v->array = n_add;
			v->capacity=newCapacity;
		}
	}
	v->array[v->size++] = val;
}

void printVector(vector *v){
	int i;
	printf("Size of vector=%d. Capcacity:%d {\n",v->size,v->capacity);
	for (i = 0; i < v->size; i++) {
		printf("%f,",v->array[i]);
	}
	printf("}\n");
}

void destroyVector(vector* v){
 	free(v->array);
	v->array = NULL;
}

/**
 * Recursive function to collect all rewards
 */
void collectRewards(treeNode* node,vector *v){
	int i;
	if (node->n <= 1) {
		pushVector(v,node->scoreSum);
		return;
	}
	for (i = 1; i < _DOM->getNumOfChildren(); i++) {
		if (node->children[i]){
			collectRewards(node->children[i],v);
		}
	}
}


/**
 * DFS to find all rewards from this node
 */
vector* allLeaves(treeNode* node){
	vector* v = createVector(10);
	collectRewards(node,v);
	return v;
}
/*** 
 * 	Compare function for qsort
 */
int rankItems(const void* a_, const void* b_){
	item* a = *(item**)a_;
	item* b = *(item**)b_;
	if (a->v > b->v){
		return 1;
	} else if (a->v == b->v){
		return 0;
	}
	return -1;
}

/**
 * Counts how many items are below set1 and set2 respectively
 */
double countItems(item** items, int size, double *set1, double *set2 ){
	int i=0;
	double sum1=0,sum2=0;
	for (i = 0; i < size; i++) {
		if(items[i]->set ==1){
			sum1+=items[i]->rank;
		}else if(items[i]->set ==2){
			sum2+=items[i]->rank;
		}
	}
	*set1=sum1;
	*set2=sum2;
	return sum1+sum2;
}

double calculateZScore(double u, int n1, int n2){
	return (u - ((double)n1*n2)/2) / sqrt((double)n1*n2*(n1+n2+1)/12);
}
void print_items(item** items,int size)
{
	int i;
	printf("-%d\n",size);
	for (i = 0; i < size; i++) {
		printf("{v=%f, s=%d, r=%f}\n",items[i]->v,items[i]->set,items[i]->rank);
	}
	puts("*");
}
	
void addRanks(item** items,int items_c){
	int i,j,k, currentRank =0, equals;
	for (i = 0; i < items_c; i+=equals) {
		for(j=i+1,equals=1;j<items_c ;j++){
			if (items[j]->v==items[i]->v)
			       	equals++;
			else
				break;
		}
		for (k = 0; k < equals; k++) {
			items[i+k]->rank = currentRank +(1/(double)equals);	
		}
		currentRank++;
	}
}

/**
 * Test if v1 is better (bigger) than v2
 */
int wilcoxon_winner_of_two(vector* v1, vector* v2) {
	int n1 = v1->size;
	int n2 = v2->size;
	if(n1==0 || n2==0){
		return 0;
	}
	int i=0,items_c=0, set_c=1;
	double u,u1,u2;
	item* items[n1+n2];
	item actualItems[n1+n2];
	//puts("v1:");
	//printVector(v1);
	//puts("v2:");
	//printVector(v2);
	for (set_c=1;set_c<=2;set_c++){
		vector* theVector = (set_c==1)?v1:v2;
		for (i=0; i< theVector->size; i++){
			actualItems[items_c].v=theVector->array[i];
			actualItems[items_c].set=set_c;
			items[items_c]=&actualItems[items_c];
			items_c++;
		}
	}
	qsort(items,items_c,sizeof(item*),rankItems);
	addRanks(items,items_c);
	countItems(items,items_c,&u1,&u2);
	//print_items(items,items_c);
	u = u1<u2? u1:u2;
	double zScore = calculateZScore(u,n1,n2);
	//printf("u1=%f , u2=%f zScore=%f ",u1,u2,zScore);
	if (zScore > Z_UPPER || zScore < Z_LOWER){
		if (u1>u2){
			return 1;
		}
		return -1;
	}
	return 0;
}



/**
 * Recieves a set of vectors and returns the winner according to wilcoxon
 * Only when one child is undoubtly better than the rest
 */
int wilcoxon_winner(vector** childData, int numOfVectors) {
	int i,j,winner=true;
	for (i = 0; i < numOfVectors; i++) {
		vector* candidate = childData[i];
		if(!candidate){
			continue;
		}
		for (j = 0; j < numOfVectors; j++) {
			if (j==i || !childData[j] ){
				continue;
			}
			assert(candidate!=childData[j]);
			int res= wilcoxon_winner_of_two(candidate,childData[j]);
			//printf(" winner-of-two=%d\n",res);
			if (res <1){
				winner=false;
				break;
			}
		}
		if (winner){
			//printf(" Return =%d\n",i);
			return i;
		}
		else winner = true;
	}
	//printf(" Return -1\n");
	return -1;
}






