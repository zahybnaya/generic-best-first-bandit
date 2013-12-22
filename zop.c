#include "zop.h"

/**
 * A mock implementation. 
 * A state is two numbers {plays:0/1 and total number}. 
 * here is a number and  
 * Zoog O Pered. 
 * Max always plays Zoog
 *
 * */

typedef struct zop_t{
	int plays; 
	int number;
} ZOP_state;

int getNumOfChildren_zop(){
	return 5;
}

int estimatedTreeSize_zop(int treeSize){
	return 2*treeSize;
}


int isValidChild_zop(rep_t rep, int side,int move){
	return true;
}

int getGameStatus_zop(rep_t rep){
	ZOP_state* s = (ZOP_state*)rep;
	if (s->plays < 2){
		return INCOMPLETE;
	}
	if (s->number%2==0)
		return MAX_WINS;
	return MIN_WINS;
}

void makeMove_zop(rep_t rep,int * side, int move){
	ZOP_state* s = (ZOP_state*)rep;
	s->plays++; 
	s->number+=move;
}


rep_t cloneRep_zop(rep_t orig){
	ZOP_state* zclone = (ZOP_state*)malloc(sizeof(ZOP_state));
	zclone->plays = ((ZOP_state*)orig)->plays;
	zclone->number = ((ZOP_state*)orig)->number;
	return zclone;

}

double applyHeuristics_zop(heuristics_t h,rep_t rep,int side, int budget){
 return 0; 
}

void destructRep_zop (rep_t rep){
 //nothing here
}

rep_t allocate_zop(){
	ZOP_state* zclone = (ZOP_state*)malloc(sizeof(ZOP_state));
	return zclone;
}



void generateRandomStart_zop(rep_t state, int side){
 ZOP_state* s = ((ZOP_state*)state);
 s->plays = 0;
 s->number= 0;
}

void copy_zop (rep_t src ,rep_t dest){
   	ZOP_state* zclone = (ZOP_state*)dest; 
	zclone->plays = ((ZOP_state*)src)->plays;
	zclone->number = ((ZOP_state*)src)->number;
}

double h1_zop(rep_t rep, int side , int move){
	return 0;
}

