#include "./gpp-reasoner/libgdl/src/gdl/logic.h"
#include "./gpp-reasoner/libgdl/src/gdl/gdl.h"
#include "./gpp-reasoner/libgdl/src/gdl/kboptimizer.h"
#include "./gpp-reasoner/libgdl/src/tools.h"
#include <string>
#include "./gpp-reasoner/libgdl/src/microtimer.h"
#include "./gpp-reasoner/libgdl/src/gdl/knowledgebase.h"
#include "./gpp-reasoner/libgdl/src/gdl/kif.h"
#include "ggp.h"

/**
 * Implementations of bridge for ggp
 * The representation should include two components: The current state and the rules.
 * */

GDL gdl;

int getNumOfChildren_ggp(rep_t rep, int side) {
	GDL::State *state = (GDL::State *)rep;
	GDL::StringVec moves = gdl.getPossibleMoves(side, *state);
	return moves.size() + 1;
}

int isValidChild_ggp(rep_t rep, int side, int move) {
	GDL::State *state = (GDL::State *)rep;
	GDL::StringVec moves = gdl.getPossibleMoves(side, *state);
		
	//We dont have a fixed amount of moves so the only way to make an illegal move would be an out of bound.
	if (move < moves.size() + 1)
		return true;
	return false;
}

int estimateTreeSize_ggp(int treeSize) { return 0; }

int getGameStatus_ggp(rep_t rep) {
	GDL::State *state = (GDL::State *)rep;
		
	if (gdl.isFinishState(*state)) {
		int result;
		gdl.points(0, *state, result);
		return result; //Anything different from INCOMPLETE is good  
	}
		
	return GGP_INCOMPLETE;	
}

void makeMove_ggp(rep_t rep, int *side, int move) {
	move--; //The system moves are 1-based while ggp is 0-based
	GDL::State *state = (GDL::State *)rep;

	GDL::StringVec aMoves = gdl.getPossibleMoves(*side, *state);
	GDL::StringVec bMoves = gdl.getPossibleMoves((*side + 1) % 2, *state);
	
	GDL::Move m;
	m.resize(2);
	m[*side] = aMoves[move];
	m[(*side + 1) % 2] = bMoves[0];

	*state = gdl.getNextState(*state, m);
	*side = (*side + 1) % 2;
}
	
rep_t cloneRep_ggp(rep_t orig) {
	GDL::State *state = (GDL::State *)orig;
	return new GDL::State(*state);
}

double applyHeuristics_ggp(heuristics_t h, rep_t rep, int side, int budget) {
	return h(rep, side, budget);
}

rep_t allocate_ggp() { 
	return new GDL::State();
}

void printBoard_ggp(rep_t rep, int dummy) {
	GDL::State *state = (GDL::State *)rep;
	std::cout << *state << std::endl;
}

void destructRep_ggp(rep_t rep) { 
	GDL::State *state = (GDL::State *)rep;
	delete state;
}

void copy_ggp(rep_t src,rep_t dst) {
	GDL::State *sSrc = (GDL::State *)src;
	GDL::State *sDst = (GDL::State *)dst;
	
	*sDst = *sSrc;
}

double h1_ggp(rep_t rep, int side, int dummy) { 
	GDL::State state = *(GDL::State *)rep;
	
	while (!gdl.isFinishState(state)) {
		GDL::StringVec aMoves = gdl.getPossibleMoves(0, state);
		GDL::StringVec bMoves = gdl.getPossibleMoves(1, state);
		GDL::Move m;
		
		m.resize(2);
		m[0] = aMoves[random() % aMoves.size()];
		m[1] = bMoves[random() % bMoves.size()];
		
		state = gdl.getNextState(state, m);
	}
	
	int result;
	gdl.points((size_t)side, state, result);
	
	return result;
}

double h2_ggp(rep_t rep, int side, int dummy) {
	return h1_ggp(rep, side, dummy) / GGP_MAX_WINS;
}

void generateRandomStart_ggp(rep_t rep, int *side) {
	GDL::State *state = (GDL::State *)rep;
	*state = gdl.getInitState();

	int i, moves = random() % 9; //TODO: param/generify to be domain dependant. for example, 9 moves is all possible in tic tac toe
	for (i = 0; i < moves; i++) {
		GDL::StringVec aMoves = gdl.getPossibleMoves(*side, *state);
		GDL::StringVec bMoves = gdl.getPossibleMoves(1 - *side, *state);
		GDL::Move m;
		
		m.resize(2);
		m[0] = aMoves[random() % aMoves.size()];
		m[1] = bMoves[random() % bMoves.size()];
		*state = gdl.getNextState(*state, m);
	}
}

void compile_ggp() {
	std::string filename = "./gpp-reasoner/hex.kif"; //TODO: Get this from argument
	KIF kif;
	
	bool result = kif.parseFile(filename, true);

	if (!result)
		std::cerr << "Error parsing " << filename << std::endl;

	gdl.initFromKIF(kif);
}