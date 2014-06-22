#include "ggp.h"
#include "./gpp-reasoner/libgdl/src/gdl/logic.h"
#include "./gpp-reasoner/libgdl/src/gdl/gdl.h"
#include "./gpp-reasoner/libgdl/src/gdl/kboptimizer.h"
#include "./gpp-reasoner/libgdl/src/tools.h"
#include <string>
#include "./gpp-reasoner/libgdl/src/microtimer.h"
#include "./gpp-reasoner/libgdl/src/gdl/knowledgebase.h"
#include "./gpp-reasoner/libgdl/src/gdl/kif.h"
#include "boost/lexical_cast.hpp"

/**
 * Implementations of bridge for ggp
 * The representation should include two components: The current state and the rules.
 * */


/*
 * Maintains the rules
 * */
struct rules {
	GDL gdl;
	GDL::State state;
	rules(GDL&,GDL::State):gdl(gdl),state(state){}
};


struct rules_holder{
	rules_holder(rules_t r): rules(r){};
	int getNumOfChildren_ggp(){ return 0; }
	int isValidChild_ggp(rep_t rep, int side, int move){
		rules_t r = static_cast<rules_t>(rep);
		GDL gdl = r->gdl;
		std::string move_s = boost::lexical_cast<std::string>(move);//moves and roles are strings 
		std::string role = boost::lexical_cast<std::string>(side);//moves and roles are strings 
		std::vector<std::string> possibleMoves = gdl.getPossibleMoves(side, r->state);
		if (std::find(possibleMoves.begin(),possibleMoves.end(),move_s) != possibleMoves.end()){
			return true;
		}
		return false;
	}
	int estimateTreeSize_ggp(int treeSize);
	int getGameStatus_ggp(rep_t rep);
	void makeMove_ggp(rep_t rep,int * side, int move);
	rep_t cloneRep_ggp(rep_t orig);
	double applyHeuristics_ggp (heuristics_t h,rep_t rep,int side, int budget);
	void generateRandomStart_ggp(rep_t rep,int*);
	rep_t allocate_ggp();
	void destructRep_ggp(rep_t rep);
	void copy_ggp(rep_t src,rep_t dst);
	double h1(rep_t rep, int side, int dummy);
	private:
	rules_t rules;

};

rules_holder* createRulesHolder(rules_t r){
	return new rules_holder(r);
}


/**
 * Parse the file and return the set of rules.
 */
rules_t compile(){
	std::string filename = "./gpp-reasoner/tictactoe.kif"; //TODO: Get this from argument
	KIF kif;
	bool result = kif.parseFile(filename, true);
	if (!result){
		std::cerr << "Error parsing " << filename << std::endl;
	}
	GDL gdl;
	gdl.initFromKIF(kif);
 return new rules(gdl, gdl.getInitState());
}



