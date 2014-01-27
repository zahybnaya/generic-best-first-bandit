#include "common.h"
#include "c4.h"

int checkWin(int *board, int *side)
{
	return (horizontalCheck(board, side) || verticalCheck(board, side) || diagonalCheck(board, side));
}

int checkFour(int *board, int a, int b, int c, int d)
{
	return (board[a] == board[b] && board[b] == board[c] && board[c] == board[d] && board[a] != -1);
}

int horizontalCheck(int *board, int *side)
{
	int row, col, idx;
	const int WIDTH = 1;

	for(row = 0; row < BOARD_ROWS; row++)
	{
		for(col = 0; col < BOARD_COLS - 3; col++)
		{
			idx = BOARD_COLS * row + col;
			if(checkFour(board, idx, idx + WIDTH, idx + WIDTH * 2, idx + WIDTH * 3))
			{
				*side = board[idx];
				return 1;
			}
		}
	}
	return 0;
}

int verticalCheck(int *board, int *side)
{
	int row, col, idx;
	const int HEIGHT = 7;

	for(row = 0; row < BOARD_ROWS - 3; row++)
	{
		for(col = 0; col < BOARD_COLS; col++)
		{
			idx = BOARD_COLS * row + col;
			if(checkFour(board, idx, idx + HEIGHT, idx + HEIGHT * 2, idx + HEIGHT * 3))
			{
				*side = board[idx];
				return 1;
			}
		}
	}
	return 0;
}

int diagonalCheck(int *board, int *side)
{
	int row, col, idx, count = 0;
	const int DIAG_RGT = 6, DIAG_LFT = 8;

	for(row = 0; row < BOARD_ROWS - 3; row++)
	{
		for(col = 0; col < BOARD_COLS; col++)
		{
			idx = BOARD_COLS * row + col;
			if((count <= 3 && checkFour(board, idx, idx + DIAG_LFT, idx + DIAG_LFT * 2, idx + DIAG_LFT * 3)) ||
					(count >= 3 && checkFour(board, idx, idx + DIAG_RGT, idx + DIAG_RGT * 2, idx + DIAG_RGT * 3)))
			{
				*side = board[idx];
				return 1;
			}
			count++;
		}
		count = 0;
	}
	return 0;
}


int pickRandomMove_connect4(int * dummyBoard, int side)
{
	int number_moves, random_move;
	number_moves = getNumOfChildren_connect4();
	random_move = random() % number_moves;

//	printf("trying random move: %d \n", random_move);
//	printBoard_c4(dummyBoard);

	while(random_move == 0 || !isValidChild_connect4(dummyBoard, side, random_move))
	{
		random_move = random() % number_moves;
		//printf("random move: %d \n", random_move);
	}

//	printf("random move: %d \n", random_move);

	return random_move;
}

static int playout_c4(int * board, int side) {
	int val;
	int dummyBoard[BOARD_ROWS * BOARD_COLS];

	// To avoid clobbering original board, we operate on a copy
	copy_connect4(board, dummyBoard);

	// Play randomly to the end
	while ((val = getGameStatus_connect4(dummyBoard)) == INCOMPLETE)
		makeMove_connect4(dummyBoard, &side, pickRandomMove_connect4(dummyBoard, side));

	// Rescale outcome to value in the set {-1, 0, +1}
	val /= MAX_WINS;

//	printf("value returned playout: %d \n", val);
//	printBoard_c4(dummyBoard);

	return val;
}

double h3_c4(rep_t rep, int side, int numPlayouts) {
	int * board =(int*)rep;
	int playoutSum = 0;
	int i;

	for (i = 0; i < numPlayouts; i++)
		playoutSum += playout_c4(board, side);

	return (double)playoutSum/(double)numPlayouts;
}

int getNumOfChildren_connect4()
{
	return BOARD_COLS + 1;
}

/*
 * currently not used
 */
int estimateTreeSize_connect4(int treeSize)
{
	return -1;
}


int isValidChild_connect4(rep_t rep, int side, int move)
{
	int * board = (int*) rep;
	int row;
	move--;

	//printf("inside isValid with move %d \n", move);

	for(row = BOARD_ROWS - 1; row >= 0; row--)
	{
		if(board[BOARD_COLS * row + move] == -1)
		{
			return 1;
		}
	}
	return 0;
}

void printBoard_c4(int *board)
{
	int row, col;

	for(row = 0; row < BOARD_ROWS; row++)
	{
		for(col = 0; col < BOARD_COLS; col++)
		{
			printf("| %d ",  board[BOARD_COLS * row + col]);
		}
		puts("|");
		puts("-----------------------------");
	}
	puts("  1   2   3   4   5   6   7\n");

}

/*
 * Max player is the one that plays first. Returns the reward for max player if it is a leaf node:
 *
 * Max wins
 * Min wins
 * Draw
 * Incomplete (not a terminal state)
 */
int getGameStatus_connect4(rep_t rep)
{
	int result, side, i;
	int * board = (int*) rep;
	side = -1;
	result = checkWin(board, &side);

//	printBoard_c4(board);

	//game has finished and variable side stores the winner
	if(result)
	{
		//MIN player wins
		if(side)
		{
//			printf("MIN WINS \n");
//			printBoard_c4(board);
			return MIN_WINS;
		}
		else //MAX player wins
		{
//			printf("MAX WINS \n");
//			printBoard_c4(board);
			return MAX_WINS;
		}
	}

	//checks if there are any moves left
	for(i = 0; i < BOARD_COLS * BOARD_ROWS; i++)
	{
		if(board[i] == -1)
		{
//			printf("GAME INCOMPLETE \n");
//			printBoard_c4(board);
			return INCOMPLETE;
		}
	}

//	printf("IT IS A DRAW \n");
//	printBoard_c4(board);
	return DRAW;
}

void makeMove_connect4(rep_t rep, int * side, int move)
{
	int * board = (int*) rep;
	int row;
	move--;

	for(row = BOARD_ROWS - 1; row >= 0; row--){
		if(board[BOARD_COLS * row + move] == -1){
			board[BOARD_COLS * row + move] = *side;
			break;
		}
	}

	*side = 1^*side;
}

rep_t cloneRep_connect4(rep_t orig)
{
	int * board_orig = orig;
	int * board = (int*) calloc(BOARD_ROWS * BOARD_COLS, sizeof(int));

	for(int i = 0; i < BOARD_ROWS * BOARD_COLS; i++)
	{
		board[i] = board_orig[i];
	}

	return board;
}

/*
 * create "playout" heuristics
 */
double applyHeuristics_connect4 (heuristics_t h, rep_t rep,int side, int budget)
{
	int * board = (int*) rep;
	return h(board, side, budget);
}

/*
 * Generate a random board by playing COLUMN * 2 moves.
 */
void generateRandomStart_connect4(rep_t rep, int* side)
{
	int random_moves, i, col, row;
	int * board = (int*) rep;

	for(row = 0; row < BOARD_ROWS; row++)
	{
		for(col = 0; col < BOARD_COLS; col++)
		{
			board[BOARD_COLS * row + col] = -1;
		}
	}

	//at most the number of columns times two moves
	random_moves = random() % (BOARD_COLS * 2);
//	printf("generating random moves to start game %d \n", random_moves);
//	printBoard_c4(board);

	for(i = 0; i < random_moves; i++)
	{
		makeMove_connect4(board, side, pickRandomMove_connect4(board, *side));
	}

	//printBoard_c4(board);
}

rep_t allocate_connect4()
{
	int row, col;
	//printf("Allocating board... \n");
	int * board = (int*) calloc(BOARD_ROWS * BOARD_COLS, sizeof(int));

	for(row = 0; row < BOARD_ROWS; row++)
	{
		for(col = 0; col < BOARD_COLS; col++)
		{
			board[BOARD_COLS * row + col] = -1;
		}
	}

	//memset(board, -1, BOARD_ROWS * BOARD_COLS);

	//printBoard_c4(board);

	return board;
}

void destructRep_connect4(rep_t rep)
{
	int* board = rep;
	free(board);
}

void copy_connect4(rep_t src, rep_t dst)
{
	int* c_src = src;
	int* c_dst = dst;

	for(int i = 0; i < BOARD_ROWS * BOARD_COLS; i++)
	{
		c_dst[i] = c_src[i];
	}
}
