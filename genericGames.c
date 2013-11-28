#include "common.h"
#include "domain.h"
#include "synth.h"
#define PRINT_CSV false
int VERBOSE = false;
int isToPrintToDot = false;
int isDotLabel = false;
int debuglog = false;
int log_this_iteration=-1;
DOM* _DOM;

static int isSuper(int firstOutcome, int secondOutcome);


int main(int argc, char* argv[]) {

  _DOM = init_domain(SYNTH);
  int maxWins=0,draws=0,minWins=0,incompletes=0,maxSuper=0,minSuper=0,same=0;
  int nodeLimit = -1;
  int i, j;
  int playedGames=0;
  int side = max;
  int rootSide;
  int origSide;
  int moveMade;
  int outcome;
  int moveCount = 0;
  Timer start;

  //TODO: Make sure this is generic too 
  double (*hOptions[])(int board[2][NUM_PITS+1],int,int) = {h1, h2, h3, h4, h5, h6}; // jump-table of possible heuristics

  //TODO: This should be renamed to get number of best moves
  int bestMoves[_DOM->getNumOfChildren()];
  int numBestMoves;


  // Variables to store command-line parameters
  int player[2]; // the search algorithms used by the max and min players TODO: make enum
  int numIterations[2] = {5000, 5000}; // number of iterations of UCT used by the max and min players
  double C[2] = {2.5, 2.5}; // exploration bias setting for UCT used by max and min players
  int budget[2] = {1, 1}; // if using playouts to estimate leaf utilities, these determine how many playouts are averaged
  short usingRandomStartBoard = true; // are we using random start boards or boards from a file?
  char boardFileName[1024]; // if using boards from a file, this contains the file name
  int numGames = 1; // number of duplicate games to play
  unsigned int seed; // seed for random number generator
  double (*heuristic[])(int board[2][NUM_PITS+1],int,int) = {h1, h1}; // the heuristics used by the max and min playersa TODO: make generic too
  short noisyHeuristic = false; // whether we will add noise to heuristic estimate
  int noiseMag = 0; // maximum magnitude of the noise that will be added
  double noiseProb = 0.0; // probability with which heuristic estimate will be corrupted
  int backupOp[2] = {MINMAX, MINMAX}; // back-up operator to be used by UCT
  int mmTreeSize[2] = {3, 3}; // back-up operator to be used by UCT TODO:enum this too

  // Variables used for pretty printing parameter settings
  char* playerStrings[] = {"max", "min"};
  char* hStrings[] = {"heuristic 1", "heuristic 2", "playouts", "random leaf values", "coarsened h1", "finer playouts"};
  char heurString[][30] = {"heuristic 1", "heuristic 1"};
  char* backupOpStrings[] = {"average", "minimax"};

  // If we see too few arguments, print help message and bail
  if (argc < 3) {
	printMessage();
  }

  // The default seed for the random number generator comes from the OS -- this may be overridden
  // if the user invokes the '-s' flag (enables repeatable runs)
  seed = (unsigned int)devrand();

  // Process command-line args
  // Mandatory args -- the search algorithms the two players will use
  for (i = 1; i <= 2; i++) {
       player[i-1] = UCT;
  }

  // Optional args -- in each case, we make sure any necessary parameter values are specified
  // and that it makes sense to define the value of this parameter given the algorithm choices
  // selected earlier (for example, trying to set the value of exploration bias for a player,
  // when that player is not using UCT will throw an error).
  for (i = 3; i < argc; i++) {
    if OPTION("-h1") {
      CHECK(max, (MINMAX | UCT | MINMAX_ON_UCT | MMUCT), "-h1")
      if (++i < argc) {
        heuristic[max] = hOptions[atoi(argv[i])-1];
        strcpy(heurString[max], hStrings[atoi(argv[i])-1]);
      }
      else
        MISSING("h1")
    }
    else if OPTION("-h2") {
      CHECK(min, (MINMAX | UCT | MINMAX_ON_UCT | MMUCT), "-h2")
      if (++i < argc) {
	heuristic[min] = hOptions[atoi(argv[i])-1];
	strcpy(heurString[min], hStrings[atoi(argv[i])-1]);
      }
      else
	MISSING("h2")
    }

    else if OPTION("-c1") {
      CHECK(max, (UCT | MINMAX_ON_UCT | MMUCT), "-c1")
      if (++i < argc)
	C[max] = atof(argv[i]);
      else
	MISSING("c1")
    }
    else if OPTION("-c2") {
      CHECK(min, (UCT | MINMAX_ON_UCT | MMUCT), "-c2")
      if (++i < argc)
	C[min] = atof(argv[i]);
      else
	MISSING("c2")
    }
    else if OPTION("-i1") {
      CHECK(max, (UCT | MINMAX_ON_UCT| MMUCT), "-i1")
      if (++i < argc)
	numIterations[max] = atoi(argv[i]);
      else
	MISSING("i1")
    }
    else if OPTION("-i2") {
      CHECK(min, (UCT | MINMAX_ON_UCT | MMUCT), "-i2")
      if (++i < argc)
	numIterations[min] = atoi(argv[i]);
      else
	MISSING("i2")
    }
    else if OPTION("-b1") {
      CHECK(max, (MINMAX | UCT | MINMAX_ON_UCT), "-b1")
      if (++i < argc)
	budget[max] = atoi(argv[i]);
      else
	MISSING("b1")
    }
    else if OPTION("-b2") {
      CHECK(min, (MINMAX | UCT | MINMAX_ON_UCT), "-b2")
      if (++i < argc)
	budget[min] = atoi(argv[i]);
      else
	MISSING("b2")
    }
    else if OPTION("-a1") {
      CHECK(max, UCT, "-a1")
      if (++i < argc)
        backupOp[max] = atoi(argv[i]) - 1;
      else
        MISSING("a1")
    }
    else if OPTION("-a2") {
      CHECK(min, UCT, "-a2")
      if (++i < argc)
        backupOp[min] = atoi(argv[i]) - 1;
      else
        MISSING("a2")
    }
    else if OPTION("-v") {
      VERBOSE = true;
    }

    else if OPTION("-l") {
      if (++i < argc) {
        strcpy(boardFileName, argv[i]);
        usingRandomStartBoard = false;
      }
      else
	MISSING("-l")
    }

    else if OPTION("-s") {
      if (++i < argc)
	seed = (unsigned int)atoi(argv[i]);
      else
	MISSING("s")
    }
    else if OPTION("-g") {
      if (++i < argc)
        numGames = atoi(argv[i]);
      else
	MISSING("g")
    }
    else if OPTION("-nh") {
      if (++i < argc) {
	noiseMag = atoi(argv[i]);
	if (++i < argc)
	  noiseProb = atof(argv[i]);
	else
	  MISSING("nh")
	noisyHeuristic = true;
      }
      else
	MISSING("nh")
    }
    else {
      printf("\nUnrecognized option %s\n", argv[i]);
      main(0, NULL);
      return 0;
    }
  }//End of argc reading

  // Cross-check parameter settings
  // If we are specifying start boards from a file, then the -g flag should not be used to avoid ambiguity
  if ((!usingRandomStartBoard) && (numGames != 1)) {
    puts("Ambiguous parameters -- only use -l or -g flag, not both");
    exit(1);
  }

  // If we want to match # of UCT iterations to that of the opposing MM player's tree size, then make sure
  // that the other player is indeed MM
  if (   ((numIterations[max] == 0) && ((player[max] == UCT) || (player[max] == MINMAX_ON_UCT)) && (player[min] != MINMAX))
      || ((numIterations[min] == 0) && ((player[min] == UCT) || (player[min] == MINMAX_ON_UCT)) && (player[max] != MINMAX))) {
    puts("Error -- cannot match compute resources of UCT player to that of a non-MM player");
    exit(1);
  }

  // Only makes sense to specify noise parameters for heuristic when at least one of the players is using h1
  if ((noisyHeuristic) && (heuristic[max] != h1) && (heuristic[min] != h1)) {
    puts("Error -- you have specified parameters for corrupting h1, yet neither player is using h1");
    exit(1);
  }

  // Seed the random number generator
  srandom(seed);

  // Print out the parameter settings (always done, regardless of verbosity level).
  printf("Parameters:\n");
  printf("Random seed: %d\n", seed);
  printf("Domain used is: ");
  printf("# Pits: %d, Stones per pit: %d\n", NUM_PITS, SHELLS_PER_PIT);
  if (noisyHeuristic)
    printf("H1 is being corrupted by noise with max. magnitude %d, probability %f\n", noiseMag, noiseProb);
  for (i = max; i <= min; i++) {
    if (player[i] == MINMAX) {
      printf("Player %d (%s) --- Minmax, Depth %d%s%s", i, playerStrings[i], 0,\
	     ((false) ? ", with random tie-breaks" : ""), ((false) ? ", with noise" : ""));
      printf(", using %s", heurString[i]);
      if ((heuristic[i] == h3) || (heuristic[i] == h6))
	printf(" (budget = %d)\n", budget[i]);
      else
	printf("\n");
    }
    else if (player[i] == UCT) {
      printf("Player %d (%s) --- UCT, C %.3f, Num iterations %d, with %s backups, using %s", i, playerStrings[i],
	     C[i], numIterations[i], backupOpStrings[i], heurString[i]);
      if ((heuristic[i] == h3) || (heuristic[i] == h6))
	printf(" (budget = %d)\n", budget[i]);
      else
	printf("\n");
    }

  fflush(stdout);
  }
  // If we are adding noise to h1, set that up now
  if (noisyHeuristic)
    setNoiseParams(noiseMag, noiseProb);
    rep_t state, randState;
    state = _DOM->allocate();
    randState = _DOM->allocate();

  /* Loops until end of games*/
  while (1) {
    if (usingRandomStartBoard) {
      _DOM->generateRandomStart(state,max);/*Sending rootSide always generates from the same max side*/
      resetTrapCounter();
    }
    // Store start board, so that we can restore it when we switch player sides
    _DOM->copy(state,randState);
    rootSide = max;
    int _outcome;
    for (j = 0; j <= 1 ; j++) {
      moveCount = 0; // reset move count
      side = rootSide; // Restore the starting board (which was either randomly generated or read from a file)
      _DOM->copy(randState,state);
      // Play complete game
      while ((outcome = _DOM->getGameStatus(state)) == INCOMPLETE) {
        origSide = side; /* this is who is currently on move -- since this is not a strict turn taking game,
			    we need to keep track of this for later bookkeeping/diagnostic messages */
        start = startTiming();
        moveMade = makeMMUCTMove(state, &side, numIterations[side], C[side], (heuristics_t)heuristic[side], budget[side], bestMoves, &numBestMoves, backupOp[side], mmTreeSize[side], nodeLimit, traps, howManyTraps);
        if (VERBOSE)
          printf("Elapsed time: %f\n", getElapsed(start));
        moveCount++;

        if (VERBOSE) {
          printf("Move #%d -- player %d made move %d\n", moveCount, origSide, moveMade);
          fflush(stdout);
        }
    } // end of game
      if (VERBOSE)
        printf("Result: %d\n", outcome/MAX_WINS);
      else
        printf("%d ", outcome/MAX_WINS);
    fflush(stdout);
    switch(outcome){
        case MAX_WINS:
            maxWins++;
            break;
        case DRAW:
            draws++;
            break;
        case MIN_WINS:
            minWins++;
            break;
        case INCOMPLETE:
            incompletes++;
            break;
        default:
            printf("ERROR: outcome is unknown");
            break;
        }
     if(j==0){
         _outcome=outcome;
     }
     if(j==1){
         int res=isSuper(_outcome,outcome);
        if(res>0){
            maxSuper++;
        } else if (res<0){
            minSuper++;
        } else
            same++;
     }

      // Now swap sides and play this same board again. This is accomplished by simply swapping the
      // parameter settings for the search algorithms (and the algorithms themselves) that the two players use
      swapInts(&player[max], &player[min]);
      swapInts(&numIterations[max], &numIterations[min]);
      swapInts(&budget[max], &budget[min]);
      swapInts(&backupOp[max], &backupOp[min]);
      swapInts(&mmTreeSize[max], &mmTreeSize[min]);
      swapDbls(&C[max], &C[min]);
      swapPtrs((void**)&heuristic[max], (void**)&heuristic[min]);
    }

    printf("\n");
    if(++playedGames>=numGames)
            break;
  }
   _DOM->destructRep(state);
   _DOM->destructRep(randState);
    free(traps);
  // Print Summary
  if(!PRINT_CSV){
      printf("------------------------------\n");
      printf("Game played: %d\n",numGames);
      printf("Draws: %d\n",same);
      printf("Incomplete games: %d\n",incompletes);
      printf("NonMatching played: %d\n",numGames-same);
      printf("Total max won games: %d\n",maxSuper);
      printf("Total min won games: %d\n",minSuper);
      printf("Max *superiority* rate: %f\n",(float)maxSuper/(maxSuper+minSuper));
      printf("Min *superiority* rate: %f\n",(float)minSuper/(maxSuper+minSuper));
      printf("Trap-gap: %d Size-of-Trap: %d # of-Traps: %d\n",trap_gap,sizeofTrap,howManyTraps);
      printf("------------------------------\n");
  }else {
      puts("Game played,Draws,Incomplete games,Max/won games,Min/won games,TrapGap,TrapSize,Number of Traps, Total won games\n");
      printf("%d,",numGames);
      printf("%d,",same);
      printf("%d,",incompletes);
      printf("%f,",(float)maxSuper/(maxSuper+minSuper));
      printf("%f,",(float)minSuper/(maxSuper+minSuper));
      printf("%d,%d,%d,",trap_gap,sizeofTrap,howManyTraps);
      printf("%d\n",maxSuper);
  }
  printMmUctStats();
  printUctStats();

  return 0;
}


static int initTrapVars(int i, char* argv[]){

        trap_gap = atoi(argv[++i]);
        sizeofTrap = atoi(argv[++i]);
        howManyTraps = atoi(argv[++i]);
        traps = (uid*)malloc(sizeof(uid)*howManyTraps);
        //TODO - check if correct
        return true;
}




/*
* is it supperrior
*/
//static int isSuper(int firstOutcome, int secondOutcome){
//    if(firstOutcome==MAX_WINS){
//        if(secondOutcome==MIN_WINS||secondOutcome==DRAW){
//            return true;
//        }
//    }
//    if(firstOutcome==DRAW){
//        if(secondOutcome==MIN_WINS){
//            return true;
//        }
//    }
//    return false;
//}

static int isSuper(int firstOutcome, int secondOutcome){
    return (firstOutcome-secondOutcome);
}
static int printMessage(){
   puts("");
   puts("Usage: games <optional-flags>");
   puts("");
   puts("Plays complete duplicate games between the two specified algorithms");
   puts("");
   puts("Algorithm Options:");
   puts("------------------");
   puts("mmuct");
   puts("");
   puts("Parameter Options:");
   puts("------------------");
   puts("-T <trap_gap> <trap_size> <how_many_traps>");
   puts("-h1/h2 <n>:    Sets heuristic for player 1/2. Values = 1 (basic), 2 (alternate), 3 (playouts), 4 (random),");
   puts("               5 (coarsened h1), 6 (finer playouts). Default = 1");
   puts("-d1/d2 <n>:    Sets depth of minimax search for player 1/2. Default = 8.");
   puts("-md1/md2 <n>:  Sets depth of minimax search for player 1/2 for MMUCT algorithm (default 3)");
   puts("-c1/c2 <n>:    Sets value of exploration bias parameter for player 1/2. Default = 0.5.");
   puts("-i1/i2 <n>:    Sets number of iterations of UCT for player 1/2. The special value of 0 makes the UCT player match it's");
   puts("               number of iterations to the number of nodes that its opponent minimax player would expand. Default = 5000.");
   puts("-b1/b2 <n>:    Sets playout budget for player 1/2. Specified number of playouts are performed at each leaf node. Default = 1.");
   puts("-a1/a2 <n>:    Back-up operator to be used for UCT player 1/2. Values = 1 (average), 2 (minimax), 3 (semimax). Default = 1.");
   puts("-n:            Makes minimax player noisy (disabled by default)");
   puts("-r:            Enables random tie-breaking for minimax player (disabled by default)");
   puts("-s <n>:        Sets seed of random number generator to n. By default, seeded with system noise.");
   puts("-g <n>:        Plays n duplicate games starting from a randomly generated starting board. Default = 1.");
   puts("-l <filename>: Loads the boards stored in the specified file one at a time and plays 1 duplicate game starting from each.");
   puts("               Not compatible with -g option.");
   puts("-nh <m> <p>:   Adds noise to the heuristic h1. The maximum magnitude of the noise is m. With probability p/2m,");
   puts("               the noise takes one of the values {+/-1, +/-2, ..., +/-m}. With probability 1-p, the magnitude of the noise");
   puts("               is 0.");
   puts("-v:            Enables VERBOSE output for tracing");
   puts("-h:            Displays this message");
   puts("");
   return 0;

}
