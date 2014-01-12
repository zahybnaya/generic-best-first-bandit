#include "mancala.h"
#include "common.h"

typedef int board_t [2][NUM_PITS+1];




/*
 implementation of the getNumOfChildren
*/
int getNumOfChildren_mancala(){
    return NUM_PITS+1;
}


/*

*/
int estimateTreeSize_mancala(int treeSize){
    return  pow(ESTIMATED_MANCALA_BRANCHING_FACTOR,treeSize);
}


/*
*
*/
int isValidChild_mancala(rep_t rep, int side, int move){
    int ** board = rep;
    int* d1Arr = board[side];
    int move_pieces = d1Arr[move];
    return (move_pieces!=0);
}


/*
*
*/

int getGameStatus_mancala(rep_t rep){

    int ** board = rep;
    TO_REGULAR_ARR(board);
    return getGameStatus(__board);
}


/*
**/
void makeMove_mancala(rep_t rep,int * side, int move){

   int **board = rep;
   TO_REGULAR_ARR(board);
   makeMove(__board,side,move);
   COPY_ARR(__board,board);
}

/*
*
**/
rep_t cloneRep_mancala(rep_t orig){

    int **orig_board = orig;
    int **board  = calloc(2,sizeof(int*));
    int i=0; for (;i<2;i++)
    board[i] = (int *)calloc(NUM_PITS+1,sizeof(int));
    COPY_ARR(orig_board,board);

   return board;
}



/*
* Write random state to rep
*/
void generateRandomStart_mancala(rep_t rep, int *side){
    int** board = rep;
    int dummy[2][NUM_PITS+1];
    genRandomBoard(dummy, side, 0);
    COPY_ARR(dummy,board);
}

/**
 copy
**/
void copy_mancala (rep_t src,rep_t dst){
    int** b_src = src;
    int** b_dst = dst;
    COPY_ARR(b_src,b_dst);
}


/*
  Allocate a new mancala board
*/
rep_t allocate_mancala(){
    int ** board = (int**)calloc(2, sizeof(int *));
    int i;
    for(i=0;i<2;i++){
        board[i]= (int*)calloc(NUM_PITS + 1, sizeof(int));
    }
    return board;
}

/*
* Destruct the board
*/
void destructRep_mancala(rep_t rep){
    int **myRep= rep;
    int i=0; for (;i<2;i++)
        free(myRep[i]);
    free(myRep);
}
/*
*
*/
double applyHeuristics_mancala (heuristics_t h,rep_t rep,int side, int budget){

    //dummy board
   int **board = rep;
   TO_REGULAR_ARR(board);

   return h(__board,side,budget);
}

