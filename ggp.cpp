#include "./gpp-reasoner/libgdl/src/gdl/logic.h"
#include "./gpp-reasoner/libgdl/src/gdl/gdl.h"
#include "./gpp-reasoner/libgdl/src/gdl/kboptimizer.h"
#include "./gpp-reasoner/libgdl/src/tools.h"
#include <string>
#include "./gpp-reasoner/libgdl/src/microtimer.h"
#include "./gpp-reasoner/libgdl/src/gdl/knowledgebase.h"
#include "./gpp-reasoner/libgdl/src/gdl/kif.h"
#include "common.h"
#include "ggp.h"

/**
 * Implementations of bridge for ggp
 * The representation should include two components: The current state and the rules.
 * */

GDL gdl;

int getNumOfChildren_ggp(rep_t rep, int side) {
	GDL::State *state = (GDL::State *)rep;
	GDL::StringVec moves = gdl.getPossibleMoves(side, *state);
	return moves.size();
}

int isValidChild_ggp(rep_t rep, int side, int move){
	GDL::State *state = (GDL::State *)rep;
	GDL::StringVec moves = gdl.getPossibleMoves(side, *state);
		
	//We dont have a fixed amout of moves so the only way to make an illegal move would be an out of bound.
	if (move < moves.size())
		return true;
	return false;
}

int estimateTreeSize_ggp(int treeSize) { return 0; }

int getGameStatus_ggp(rep_t rep) {
	GDL::State *state = (GDL::State *)rep;
		
	if (gdl.isFinishState(*state))
		return MAX_WINS; //Anything different from INCOMPLETE is good  
		
	return INCOMPLETE;	
}

void makeMove_ggp(rep_t rep, int *side, int move) {
	GDL::State *state = (GDL::State *)rep;

	GDL::StringVec aMoves = gdl.getPossibleMoves(*side, *state);
	GDL::StringVec bMoves = gdl.getPossibleMoves((*side + 1) % 2, *state);
	
	GDL::Move m;
	m.resize(2);
	m[0] = aMoves[move];
	m[1] = bMoves[0];

	*state = gdl.getNextState(*state, m);
	*side = (*side + 1) % 2;
}
	
rep_t cloneRep_ggp(rep_t orig) {
	rep_t clone = calloc(1, sizeof(GDL::State));
	memcpy(clone, orig, sizeof(GDL::State));
	return clone;
}

double applyHeuristics_ggp(heuristics_t h, rep_t rep, int side, int budget) {
	return h(rep, side, budget);
}

void generateRandomStart_ggp(rep_t rep, int *side) {
  
}

rep_t allocate_ggp();
void destructRep_ggp(rep_t rep);
void copy_ggp(rep_t src,rep_t dst);
double h1(rep_t rep, int side, int dummy);

void compile_ggp() {
	std::string filename = "./gpp-reasoner/tictactoe.kif"; //TODO: Get this from argument
	KIF kif;
	
	bool result = kif.parseFile(filename, true);

	if (!result)
		std::cerr << "Error parsing " << filename << std::endl;

	gdl.initFromKIF(kif);
}