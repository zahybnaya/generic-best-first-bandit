
#include "common.h"
#include "mancala.h" 

/* Set of routines that operate on boards */

/* Boards are represented by a 2-d array of numbers. First index (0 = max, 1 = min)
   refers to the side of the board. Second index (0 = store, 1 ... NUM_PITS = other pits)
   refers to the index of the pit. The value held is the number of stones in that pit.

   For example, consider the following board:

   Max  8 0 4 3 3 2 7
   Min*   4 1 2 0 1 3 10

   This would be internally represented as follows:
   board[0][0] = board[max][store] = 8
   board[max][1] = 0; board[max][2] = 4 etc.
   board[1][0] = board[min][store] = 10
   board[min][1] = 3; board[min][2] = 1 etc.

   A move is simply represented by an integer representing the pit index. Thus, if Min
   wants to make the move that would empty the pit containing the 4 stones, this move
   would simply be represented as 6. */


/* Sets the board configuration to that of the the starting board in Mancala */
void initBoard(int board[2][NUM_PITS+1], int* side) {
  int i;

  board[max][store] = board[min][store] = 0;
  for (i = 1; i < NUM_PITS+1; i++) {
    board[max][i] = SHELLS_PER_PIT;
    board[min][i] = SHELLS_PER_PIT;
  }

  *side = max;
}


/* Generates a random starting configuration for the game at the specified depth */
void genRandomBoard(int board[2][NUM_PITS+1], int* side, int depth) {
  int j;
  int pitCount;
  int numBestMoves;
  int bestMoves[NUM_PITS];
  double termPercentage;

  do {
    // First, blank out the board
    board[max][store] = board[min][store] = 0;
    for (j = 1; j < NUM_PITS+1; j++) {
      board[max][j] = 0;
      board[min][j] = 0;
    }

    pitCount = NUM_PITS * SHELLS_PER_PIT * 2; // total number of stones that need to be placed

    // Place the stones uniformly at random on the board (we don't place stones in stores)
    while (pitCount > 0) {
      board[random() % 2][(random() % NUM_PITS) + 1] += 1; // place a stone uniformly at random in one of the pits
      pitCount--;
    }

    // Randomly choose who goes first
    *side = random() % 2;

    // Now play the game from this position to the specified depth using two MM-12-H players
    for (j = 0; j < depth; j++)
      if (getGameStatus(board) == INCOMPLETE)
	makeMinmaxMove(board,  side, 12, (heuristics_t)h1, 0, false, false, bestMoves, &numBestMoves, &termPercentage);
  } while (getGameStatus(board) != INCOMPLETE); // if the game has already ended, the board is useless so start over
}

/* Copies srcBoard --> dstBoard */
void cloneBoard(int srcBoard[2][NUM_PITS+1], int dstBoard[2][NUM_PITS+1]) {
  int i;

  for (i = 0; i < NUM_PITS+1; i++) {
    dstBoard[max][i] = srcBoard[max][i];
    dstBoard[min][i] = srcBoard[min][i];
  }
}


/* Opens the specified file, and loads the board stored in it on the first call (in READ mode). Subsequent calls
   return the next board in the file (only one board per line). Returns true if a board could be read, and false
   when EOF is reached. If called in CLOSE mode, the file is closed. Any board formatting errors cause the program
   to quit.

   Board format is as follows:
   <Side> <Max's store> <Max's pits starting from pit 1> <Min's store> <Min's pits starting from pit 1>

   For example, the following board in pretty-print format:
     Max  8 0 4 3 3 2 7
     Min*   4 1 2 0 1 3 10

   would be stored in the file as:
   1 8 0 4 3 3 2 7 10 3 1 0 2 1 4

   Values for the side on move field are 0 to indicate Max's move, and 1 to indicate Min's move */
int readBoard(char* fileName, int board[2][NUM_PITS+1], int* side, short mode) {
  static FILE* f = NULL;
  static char fileNameCopy[1024];
  int temp;
  int totalStoneCount = 0;
  int i, j;

  // Only two legal modes
  assert((mode == READ) || (mode == CLOSE));

  // In this mode, we simply close the file and return
  if (mode == CLOSE) {
    if (f) {
      fclose(f);
      f = NULL;
      strcpy(fileNameCopy, "");
      return true;
    }

    return false;
  }

  // Otherwise, in READ mode
  if (!f) { // is this the first read call? If so, open the file
    f = fopen(fileName, "r");

    if (!f) { // couldn't read from file
      printf("Error -- couldn't open file %s\n", fileName);
      exit(1);
    }

    strcpy(fileNameCopy, fileName); // save the name of the file we just opened
  }

  // Make sure we are reading the same file as before -- switching between different files is not allowed
  assert(!strcmp(fileName, fileNameCopy));

  // Have we hit EOF? If so, there is nothing left to read, so return false
  if (feof(f))
    return false;

  // Read the side on move field
  if (fscanf(f, "%d", &temp) == 1) { // successful read
    // Validate input
    if (temp == 0)
      *side = max;
    else if (temp == 1)
      *side = min;
    else {
      puts("Error reading side on move");
      exit(1);
    }
  }
  else {
    puts("Error reading side on move");
    exit(1);
  }

  // Read in the rest of the board
  for (i = 0; i < 2; i++) { // for each player
    for (j = 0; j < NUM_PITS+1; j++) { // for each pit (and store)
      if (fscanf(f, "%d", &temp) == 1) { // successful read
	board[i][j] = temp;
	totalStoneCount += temp;
      }
      else {
	puts("Error reading board");
	exit(1);
      }
    }
  }

  // Make sure that the total number of stones in the pits equals what is expected
  if (totalStoneCount != (2 * NUM_PITS * SHELLS_PER_PIT)) {
    puts("Error -- invalid board");
    exit(1);
  }

  // Consume the newline
  fscanf(f, "\n");

  return true;
}


/* Opens the specified file, and writes the current board to it on the first call. Subsequent
   calls append a new board to the already open file.
*/
void writeBoard(char* fileName, int board[2][NUM_PITS+1], int side, short mode) {
  static FILE* f = NULL;
  static char fileNameCopy[1024];
  int i, j;

  // Only two legal modes
  assert((mode == WRITE) || (mode == CLOSE));

  // In this mode, just close the file and return
  if (mode == CLOSE) {
    if (f) {
      fclose(f);
      f = NULL;
      strcpy(fileNameCopy, "");
    }

    return;
  }

  // Otherwise, in WRITE mode
  if (!f) { // is this the first write call? If so, open the specified file
    f = fopen(fileName, "w");

    if (!f) { // could not open file
      puts("Error -- could not write to specified file");
      exit(1);
    }

    strcpy(fileNameCopy, fileName); // save the name of the file we just opened
  }

  fprintf(f, "%d ", side); // write side on move

  // Write the rest of the board -- see format details in header of readBoard()
  for (i = 0; i < 2; i++)
    for (j = 0; j < NUM_PITS+1; j++)
      fprintf(f, "%d ", board[i][j]);
  fprintf(f, "\n");

}


/* Determines if board1 == board2 */
int isEqual(int board1[2][NUM_PITS+1], int board2[2][NUM_PITS+1]) {
  int i;

  for (i = 0; i < NUM_PITS+1; i++)
    if ((board1[max][i] != board2[max][i]) || (board1[min][i] != board2[min][i]))
      return false;

  return true;
}


/* Utility to pretty print the given board. Side on move is indicated with a '*' */
void printBoard_mancala(rep_t rep, int side) {
  int **board = rep;
  TO_REGULAR_ARR(board);

  int i;
  printf("%s: ", (side == max) ? "*Max" : " Max");
  printf("%2d ", __board[max][store]);
  for (i = 1; i < NUM_PITS+1; i++)
    printf("%d ", __board[max][i]);
  printf("\n");

  printf("%s:    ", (side == min) ? "*Min" : " Min");
  for (i = NUM_PITS; i >= 0; i--)
    printf("%d ", __board[min][i]);
  printf("\n");
}
