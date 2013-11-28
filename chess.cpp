
#include "chess.h"
#include <iostream>
#include "engine/move.h"
#include "engine/board.h"
#define MAX_BF 40
/*
 implementation of the getNumOfChildren
*/
int getNumOfChildren_chess(){
    return MAX_BF;
}

/*

*/
int estimateTreeSize_chess(int treeSize){

}


/*
*
*/
int isValidChild_chess(rep_t rep, int side, int move){

}

/*
*
*/

int getGameStatus_chess(rep_t rep){

}


/*
**/
void makeMove_chess(rep_t rep,int * side, int move){

}
/*
*
**/
rep_t cloneRep_chess(rep_t orig){

}



/*
* Write random state to rep
*/
void generateRandomStart_chess(rep_t rep,int side){



}

/**
 copy
**/
void copy_chess (rep_t src,rep_t dst){

}


/*
  Allocate a new chess board
*/
rep_t allocate_chess(){

}
/*
* Destruct the board
*/
void destructRep_chess(rep_t rep){

}
/*
*
*/
double applyHeuristics_chess (heuristics_t h,rep_t rep,int side, int budget){

}

int main(){
    engine::board_t board;
    std::cout<<"Hello"<<std::endl;
    board_clear(&board);

}
