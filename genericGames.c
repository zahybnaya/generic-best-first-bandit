#include "common.h"
#include "domain.h"
#include "synth.h"
#include <string.h>
#define PRINT_CSV true
int verbose = false;
int isToPrintToDot = false;
int isDotLabel = false;
int debuglog = false;
int log_this_iteration=-1;
int logIteration=0;
DOM* _DOM;
 
static int isSuper(int firstOutcome, int secondOutcome);
static int printMessage();
static char* createAlgorithmDecription(int player[],int player_side);

/*
 * ./ggames 4 b u -a1 1 -a2 1 -ts1 2 -t1 500 -h1 3 -h2 3 -c1 2.5 -c2 2.5 -i1 5000 -i2 5000 -s 0 -g 30
 */


int main(int argc, char* argv[]) {

  int maxWins=0,draws=0,minWins=0,incompletes=0,maxSuper=0,minSuper=0,same=0;
  int i, j;
  int playedGames=0;
  int side = max;
  int rootSide;
  int origSide;
  int moveMade;
  int outcome;
  int moveCount = 0;
  Timer start;
  int numBestMoves;

  // Variables to store command-line parameters
  int player[2]; // the search algorithms used by the max and min players TODO: make enum
  int depth[2] = {8, 8}; // the search depth cutoff (only relevant to MM players)
  int noisyMM = false; // determine whether the MM player will play noisily (i.e. occasionally pick the second best move)
  double termPercentage;
  int randomTieBreaks = false; // determine whether the MM player will break ties randomly
  int numIterations[2] = {5000, 5000}; // number of iterations of UCT used by the max and min players
  double C[2] = {2.5, 2.5}; // exploration bias setting for UCT and BFB used by max and min players
  double CT[2] = {2.5, 2.5}; // exploration bias setting for BFB types used by max and min players
  int budget[2] = {1, 1}; // if using playouts to estimate leaf utilities, these determine how many playouts are averaged
  short usingRandomStartBoard = true; // are we using random start boards or boards from a file?
  char boardFileName[1024]; // if using boards from a file, this contains the file name
  int numGames = 1; // number of duplicate games to play
  unsigned int seed; // seed for random number generator
  short noisyHeuristic = false; // whether we will add noise to heuristic estimate
  int noiseMag = 0; // maximum magnitude of the noise that will be added
  double noiseProb = 0.0; // probability with which heuristic estimate will be corrupted
  int backupOp[2] = {MINMAX, MINMAX}; // back-up operator to be used by UCT
  int mmTreeSize[2] = {3, 3}; // back-up operator to be used by UCT TODO:enum this too
  int type_system[2] = {STS, STS}; //the type system used by BFB.
  int threshold[2] = {100, 100}; //the maximal number of nodes in a type for the STS type system
  int policy[2] = {MAB, MAB}; //the policy used by BFB to choose types.
  double time[2] = {0.0, 0.0}; //the total time each player took to play across all games.
  int turns[2] = {0, 0}; //the total number of turns each player played across all games.
  
  // Variables used for pretty printing parameter settings
  const char* playerStrings[] = {"max", "min"};
  const char* hStrings[] = {"heuristic 1", "heuristic 2", "playouts", "random leaf values", "coarsened h1", "finer playouts"};
  char heurString[][30] = {"heuristic 1", "heuristic 1"};
  const char* backupOpStrings[] = {"average", "minimax"};

  // If we see too few arguments, print help message and bail
  if (argc < 3) {
	printMessage();
	return (-1);
  }

  // The default seed for the random number generator comes from the OS -- this may be overridden
  // if the user invokes the '-s' flag (enables repeatable runs)
  seed = (unsigned int)devrand();

  /*Domain name initializaiont*/
  int dom_name = atoi(argv[1]);
  switch(dom_name){
	  case 0:
		  _DOM = init_domain(MANCALA);
		  break;
	  case 1:
		  _DOM = init_domain(SYNTH);
		  break;
	  case 2:
		  _DOM = init_domain(CHESS);
		  break;

	  case 3:
		  _DOM = init_domain(ZOP);
		  break;
	  case 4:
		  _DOM = init_domain(C4);
		  break;
	  default:
		  puts("Unrecognized domain description.");
		  return (-1);
  }
  int bestMoves[_DOM->getNumOfChildren()];
  heuristics_t heuristic[] =  {_DOM->hFunctions.h1, _DOM->hFunctions.h1}; // the heuristics used by the max and min playersa 

  // Process command-line args
  // Mandatory args -- the search algorithms the two players will use
  for (i = 1; i <= 2; i++) {
    if (strcmp(argv[i + 1], "m") == 0)
      player[i-1] = MINMAX;
    /*Zahy*/
    else if (strcmp(argv[i + 1], "mmuct") == 0)
      player[i-1] = MMUCT;
    else if (strcmp(argv[i + 1], "u") == 0)
      player[i-1] = UCT;
    else if (strcmp(argv[i + 1], "r") == 0)
      player[i-1] = RANDOM;
    else if (strcmp(argv[i + 1], "mu") == 0)
      player[i-1] = MINMAX_ON_UCT;
    else if (strcmp(argv[i + 1], "b") == 0)
      player[i-1] = BFB;
    else {
      printf("Unrecognized algorithm choice %s", argv[i + 1]);
      return 0;
    }
  }
  
  // Optional args -- in each case, we make sure any necessary parameter values are specified
  // and that it makes sense to define the value of this parameter given the algorithm choices
  // selected earlier (for example, trying to set the value of exploration bias for a player,
  // when that player is not using UCT will throw an error).
  for (i = 4; i < argc; i++) {
    if OPTION("-h1") {
      CHECK(max, (MINMAX | UCT | MINMAX_ON_UCT | MMUCT | BFB), "-h1")
      if (++i < argc) {
	switch(atoi(argv[i])) {
	  case 1:
	    heuristic[max] = _DOM->hFunctions.h1;
	    break;
	  case 2:
	    heuristic[max] = _DOM->hFunctions.h2;
	    break;
	  case 3:
	    heuristic[max] = _DOM->hFunctions.h3;
	    break;
	  case 4:
	    heuristic[max] = _DOM->hFunctions.h4;
	    break;
	  case 5:
	    heuristic[max] = _DOM->hFunctions.h5;
	    break;
	  case 6:
	    heuristic[max] = _DOM->hFunctions.h6;
	    break;
	}
        
        strcpy(heurString[max], hStrings[atoi(argv[i])-1]);
      }
      else
        MISSING("h1")
    }
    else if OPTION("-h2") {
      CHECK(min, (MINMAX | UCT | MINMAX_ON_UCT | MMUCT | BFB), "-h2")
      if (++i < argc) {
	switch(atoi(argv[i])) {
	  case 1:
	    heuristic[min] = _DOM->hFunctions.h1;
	    break;
	  case 2:
	    heuristic[min] = _DOM->hFunctions.h2;
	    break;
	  case 3:
	    heuristic[min] = _DOM->hFunctions.h3;
	    break;
	  case 4:
	    heuristic[min] = _DOM->hFunctions.h4;
	    break;
	  case 5:
	    heuristic[min] = _DOM->hFunctions.h5;
	    break;
	  case 6:
	    heuristic[min] = _DOM->hFunctions.h6;
	    break;
	}
	strcpy(heurString[min], hStrings[atoi(argv[i])-1]);
      }
      else
	MISSING("h2")
    }

    else if OPTION("-ts1") {
      CHECK(max, BFB, "-ts1")
      if (++i < argc)
	type_system[max] = atoi(argv[i]);
      else
	MISSING("ts1")
    }
    else if OPTION("-ts2") {
      CHECK(min, BFB, "-ts2")
      if (++i < argc)
	type_system[min] = atoi(argv[i]);
      else
	MISSING("ts2")
    }
    else if OPTION("-t1") {
      CHECK(max, BFB, "-t1")
      if (++i < argc)
	threshold[max] = atoi(argv[i]);
      else
	MISSING("t1")
    }
    else if OPTION("-t2") {
      CHECK(min, BFB, "-t2")
      if (++i < argc)
	threshold[min] = atoi(argv[i]);
      else
	MISSING("t2")
    }
    else if OPTION("-p1") {
      CHECK(max, BFB, "-p1")
      if (++i < argc)
	policy[max] = atoi(argv[i]);
      else
	MISSING("p1")
    }
    else if OPTION("-p2") {
      CHECK(min, BFB, "-p2")
      if (++i < argc)
	policy[min] = atoi(argv[i]);
      else
	MISSING("p2")
    }
    
    
    else if OPTION("-c1") {
      CHECK(max, (UCT | MINMAX_ON_UCT | MMUCT | BFB), "-c1")
      if (++i < argc)
	C[max] = atof(argv[i]);
      else
	MISSING("c1")
    }
    else if OPTION("-c2") {
      CHECK(min, (UCT | MINMAX_ON_UCT | MMUCT | BFB), "-c2")
      if (++i < argc)
	C[min] = atof(argv[i]);
      else
	MISSING("c2")
    }
    
    else if OPTION("-ct1") {
      CHECK(max, BFB, "-ct1")
      if (++i < argc)
	CT[max] = atof(argv[i]);
      else
	MISSING("ct1")
    }
    else if OPTION("-ct2") {
      CHECK(min, BFB, "-ct2")
      if (++i < argc)
	CT[min] = atof(argv[i]);
      else
	MISSING("ct2")
    }
    else if OPTION("-i1") {
      CHECK(max, (UCT | MINMAX_ON_UCT| MMUCT | BFB), "-i1")
      if (++i < argc)
	numIterations[max] = atoi(argv[i]);
      else
	MISSING("i1")
    }
    else if OPTION("-i2") {
      CHECK(min, (UCT | MINMAX_ON_UCT | MMUCT | BFB), "-i2")
      if (++i < argc)
	numIterations[min] = atoi(argv[i]);
      else
	MISSING("i2")
    }
    
    else if OPTION("-d1") {
      CHECK(max, MINMAX, "-d1")
      if (++i < argc)
	depth[max] = atoi(argv[i]);
      else
	MISSING("d1")
    }
    else if OPTION("-d2") {
      CHECK(min, MINMAX, "-d2")
      if (++i < argc)
	depth[min] = atoi(argv[i]);
      else
	MISSING("d2")
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
      CHECK(max, (UCT | BFB), "-a1")
      if (++i < argc)
        backupOp[max] = atoi(argv[i]) - 1;
      else
        MISSING("a1")
    }
    else if OPTION("-a2") {
      CHECK(min, (UCT | BFB), "-a2")
      if (++i < argc)
        backupOp[min] = atoi(argv[i]) - 1;
      else
        MISSING("a2")
    }
    else if OPTION("-v") {
      verbose = true;
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
  if ((noisyHeuristic) && (heuristic[max] !=_DOM->hFunctions.h1) && (heuristic[min] != _DOM->hFunctions.h1)) {
    puts("Error -- you have specified parameters for corrupting h1, yet neither player is using h1");
    exit(1);
  }

  // Seed the random number generator
  srandom(seed);
  
  // Print out the parameter settings (always done, regardless of verbosity level).
  printf("Parameters:\n");
  printf("Random seed: %d\n", seed);
  printf("Domain used is: %d\n",_DOM->dom_name);
  printf("# Pits: %d, Stones per pit: %d\n", NUM_PITS, SHELLS_PER_PIT); //TODO: domain specific parameters.
  if (noisyHeuristic)
    printf("H1 is being corrupted by noise with max. magnitude %d, probability %f\n", noiseMag, noiseProb);
  for (i = max; i <= min; i++) {
    if (player[i] == MINMAX) {
      printf("Player %d (%s) --- Minmax, Depth %d%s%s", i, playerStrings[i], depth[i],\
	     ((false) ? ", with random tie-breaks" : ""), ((false) ? ", with noise" : ""));
      printf(", using %s", heurString[i]);
      if ((heuristic[i] == _DOM->hFunctions.h3) || (heuristic[i] == _DOM->hFunctions.h6))
	printf(" (budget = %d)\n", budget[i]);
      else
	printf("\n");
    }
    else if (player[i] == UCT) {
      printf("Player %d (%s) --- UCT, C %.3f, Num iterations %d, with %s backups, using %s", i, playerStrings[i],
	     C[i], numIterations[i], backupOpStrings[backupOp[i]], heurString[i]);
      if ((heuristic[i] == _DOM->hFunctions.h3) || (heuristic[i] == _DOM->hFunctions.h6))
	printf(" (budget = %d)\n", budget[i]);
      else
	printf("\n");
    }
    else if (player[i] == BFB) {
      printf("Player %d (%s) --- BFB, C %.3f, Num iterations %d, with %s backups, using %s", i, playerStrings[i],
	     C[i], numIterations[i], backupOpStrings[backupOp[i]], heurString[i]);
      if ((heuristic[i] == _DOM->hFunctions.h3) || (heuristic[i] == _DOM->hFunctions.h6))
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
      _DOM->generateRandomStart(state,&side);/*Sending rootSide always generates from the same max side*/
      //resetTrapCounter();
    }
    // Store start board, so that we can restore it when we switch player sides
    _DOM->copy(state,randState);
    rootSide = side;

    int _outcome;
    for (j = 0; j <= 1 ; j++) {
      
      moveCount = 0; // reset move count
      side = rootSide; // Restore the starting board (which was either randomly generated or read from a file)
      _DOM->copy(randState,state);
      // Play complete game
      while ((outcome = _DOM->getGameStatus(state)) == INCOMPLETE) {
        origSide = side; /* this is who is currently on move -- since this is not a strict turn taking game,
			    we need to keep track of this for later bookkeeping/diagnostic messages */
        if (verbose) {
	  printf("\n");
	  printBoard(state, side);
	  fflush(stdout);
	}
        
        start = startTiming();
	//Only UCT for now
	switch(player[side]){
		case UCT:
			moveMade = makeUCTMove(state, &side, numIterations[side], C[side], heuristic[side], budget[side], bestMoves, &numBestMoves, backupOp[side]);
			break;
		case MINMAX:
			moveMade = makeMinmaxMove(state, &side,depth[side],heuristic[side],budget[side],randomTieBreaks,noisyMM,bestMoves,&numBestMoves, &termPercentage);
			break;
		case BFB:
			moveMade = makeBFBMove(state, &side, type_system[side], numIterations[side], C[side], CT[side], heuristic[side], budget[side], bestMoves, &numBestMoves, backupOp[side], threshold[side], policy[side]);
			break;
		default:
			puts("Unknown algorithm\n");
	}

        if (verbose)
          printf("Elapsed time: %f\n", getElapsed(start));
	
	turns[side]++;
	time[side] = time[side] + getElapsed(start);
	
        moveCount++;
        if (verbose) {
          printf("Move #%d -- player %d made move %d\n", moveCount, origSide, moveMade);
          fflush(stdout);
        }
    } // end of game
    
      if (verbose) {
	printBoard(state, side);
	printf("\n");
        printf("Result: %d\n", outcome/MAX_WINS);
      } else
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
      swapDbls(&CT[max], &CT[min]);
      swapPtrs((void**)&heuristic[max], (void**)&heuristic[min]);
      swapInts(&type_system[max], &type_system[min]);
      swapInts(&threshold[max], &threshold[min]);
      swapInts(&policy[max], &policy[min]);
      swapInts(&depth[max], &depth[min]);
      swapDbls(&time[max], &time[min]);
      swapInts(&turns[max], &turns[min]);
    }

    printf("\n");
    if(++playedGames>=numGames)
            break;
  }
   _DOM->destructRep(state);
   _DOM->destructRep(randState);
   char* algDescription []={createAlgorithmDecription(player,max),createAlgorithmDecription(player,min)};

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
      printf("------------------------------\n");
  }else {
      puts("domain, max_alg, min_alg ,num_of_games,num_of_draws,num_of_incomplete,max_win,min_win,total_win_games,max_time,min_time\n");
      printf("%d,",_DOM->dom_name);
      printf("%s,",algDescription[max]);
      printf("%s,",algDescription[min]);
      printf("%d,",numGames);
      printf("%d,",same);
      printf("%d,",incompletes);
      printf("%f,",(float)maxSuper/(maxSuper+minSuper));
      printf("%f,",(float)minSuper/(maxSuper+minSuper));
      printf("%d,",maxSuper);
      printf("%f,",time[max]/(double)turns[max]);
      printf("%f\n",time[min]/(double)turns[min]);
  }
  printUctStats();
  free(algDescription[0]);
  free(algDescription[1]);

  return 0;
}

static int isSuper(int firstOutcome, int secondOutcome){
    return (firstOutcome-secondOutcome);
}

static int printMessage(){
   puts("");
   puts("Usage: games <DOMAIN> <optional-flags>");
   puts("");
   puts("Plays complete duplicate games between the two specified algorithms");
   puts("");
   puts("Note! not all options are supported");
   puts("Available DOMAIN names:");
   puts("------------------");
   puts("(0) MANCALA (1) SYNTH (2) CHESS (3) ZOP"); 
   puts("Algorithm Options:");
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
   puts("-ts1/ts2:      Set the type system for bfb player 1/2. Values = 1 (Oracle), 2 (STS)");
   puts("-t1/t2:        Set the maximal size for bfb player 1/2 using STS type system. Default = 100");
   puts("-p1/p2:        The policy used by player 1/2 for choosing types in BFB. Values = 1 (MAB), 2 (VMAB). Default = 1.");
   puts("-v:            Enables VERBOSE output for tracing");
   puts("-h:            Displays this message");
   puts("");
   return 0;
}

/**
 * describe an algorithm
 * */
static char* createAlgorithmDecription(int player[],int playerInd){
 char* ret = (char*)malloc(sizeof(char)*128);
 switch (player[playerInd]) {
   case MINMAX:
     sprintf(ret,"%s", "MINMAX");
     break;
   case UCT:
     sprintf(ret, "%s", "UCT");
     break;
   case BFB:
     sprintf(ret, "%s", "BFB");
     break;
 }
 return ret;
}
