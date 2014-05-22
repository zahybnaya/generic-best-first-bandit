/*
 * main.cpp
 *
 *  Created on: 11.11.2008
 *      Author: nos
 */

// #include "gdl/gdl.h"
#include "gdl/kif.h"
#include "gdl/logic.h"
#include "gdl/gdl.h"
#include "gdl/kboptimizer.h"
#include "tools.h"
#include <string>
#include "microtimer.h"
#include "gdl/knowledgebase.h"

unsigned int maxAnswers = 100;

/// Prints out a logic answer; with backtrace if you want
void dumpAnswer (Answer & answer, bool backtrace = false){
	bool hasOneTrue = false;
	unsigned int i = 0;
	while (answer.next()){
		std::cout << "True, " << answer.getQuestionMap() << std::endl;

		if (backtrace){
			answer.backtrace (std::cout);
			std::cout << std::endl;
			std::cout << std::endl;
		}

		hasOneTrue = true;
		i++;
		if (i > maxAnswers) {
			std::cout << "Stopping as max answer count is reached" << std::endl;
			return;
		}
	}
	if (!hasOneTrue) std::cout << "False " << std::endl;
}

/// Asks the Logic/KnowledgeBase something and prints the answer
void printAsk (Logic & l, const std::string & question, const KnowledgeBase & kb, bool backtrace = false){
	std::cout << "Asking the logic full system the following question " << question << std::endl;
	Answer * answer = l.ask (question, kb);
	dumpAnswer (*answer, backtrace);
	delete answer;
}

/// libgdl testcases
int main (int argc, char * argv[]){
	/*
	 * The testcase consists of some testcases of the logic itself and some of the GDL abstraction.
	 */
	
	if (argc != 2){
		std::cout << "Usage parser NameOfFile" << std::endl;
	}

	std::cout << "Reading in KIF: " << std::endl;
	std::string filename = argv[1];
	KIF kif;
	bool result = kif.parseFile(filename, true);
	if (!result){
		std::cerr << "Error parsing " << filename << std::endl;
	}
	
	KnowledgeBase kb (kif);

	
	// LOGIC TESTCASES
	Logic logic;
	std::cout << "Logic testcases " << std::endl;
	std::cout << "Knowledge Base size: " << kb.size() << std::endl;
	logic.setDebugLevel (2);
	printAsk (logic, "(role ?A)", kb, false);
	logic.setDebug (false);
	printAsk (logic, "(init ?A)", kb, false);

	// testcase for head, tail
	{
		std::cout << "** Head/Tail Testcase **" << std::endl;
		std::string testString = "(a b c)";
		std::string head, tail;
		KIF::splitHeadTail (testString, head, tail);
		if (head != "a" || tail != "(b c)") std::cout << LOGID << "Head/Tail testcase failed head: " << head << " tail: " << tail << std::endl;
		std::string testString2 = "((abcd))";
		KIF::splitHeadTail (testString2, head, tail);
		if (head != "(abcd)" || tail != "()") std::cout << LOGID << "Head/Tail testcase failed head: " << head << " tail: " << tail << std::endl;
	}
	
	// double variable Instance
	{
		std::cout << "** Double Variable Instance Testcase **" << std::endl;
		KnowledgeBase myKb;
		myKb.add ("(<= (foo ?x ?y) (bar ?x ?y))");
		myKb.add ("(bar ?x ?x)");
		// logic.setDebug (true);
		// std::cout << " KB: " << myKb << std::endl;
		Logic::StringVec result = logic.simpleAsk("(foo ?x 1)", "?x", myKb);
		if (result.size()!=1 || result[0] != "1"){
			std::cerr << LOGID << "[FAILED; returned " << result << "]" << std::endl;
		}
		// printAsk (logic, "(foo ?x 1)", myKb, true);
		// logic.setDebug (false);
	}
	
	// KnowledegMap self inspection 
	{
		std::cout << "** KnowledgeMap optimization testcase **" << std::endl;
		KBOptimizer optimizer (kb);
		optimizer.optimizeOrder();				// 
		optimizer.optimizeSpecialRelations ();
	}
	
	// GDL TESTCASES
	GDL gdl;
	gdl.setCompressing (false); // when you enable this; you won't be able to read the results of the GDL; before translating them with GDL::decompress
	if (!gdl.initFromKIF(kif)){
		std::cerr << "Could not init GDL from KIF" << std::endl;
		return 1;
	}
//	{
//		std::cout << "Mass Copy test" << std::endl;
//		std::vector<GDL> v(1000);
//		uint64_t before = microtimer ();
//		for (int i = 0; i < 1000; i++) { v[i] = gdl; }
//		uint64_t after = microtimer ();
//		std::cout << "It took " << (after - before) << "microsec" << std::endl;
//	}

	
	// gdl.setDebug ();

	uint64_t timeBegin = microtimer ();
	GDL::StringVec roles = gdl.getRoles ();
	std::cout << "Roles: " << roles << std::endl;
	GDL::State initState = gdl.getInitState ();
	std::cout << "Init State: " << gdl.getInitState () << std::endl;
	for (size_t i = 0; i < gdl.getRoleCount(); i++){
		std::cout << "Possible Moves for " << roles[i] << ": " <<  gdl.getPossibleMoves (i, initState) << std::endl;
	}

	// for roles count == 1
	if (roles.size() == 1){
		GDL::StringVec moves = gdl.getPossibleMoves (0, initState);
		for (unsigned int i = 0; i < moves.size(); i++){
			GDL::Move m;
			m.resize (1);
			m[0] = moves[i];

			GDL::State nextState = gdl.getNextState (initState, m);

			std::cout << "Next State after " << moves[i] << " " << nextState << std::endl;
		}
	}

	// for roles count == 2
	if (roles.size() == 2){
		GDL::StringVec aMoves = gdl.getPossibleMoves (0, initState);
		GDL::StringVec bMoves = gdl.getPossibleMoves (1, initState);
		for (unsigned int i = 0; i < aMoves.size(); i++){
			for (unsigned int j = 0; j < bMoves.size(); j++){
				GDL::Move m;
				m.resize (2);
				m[0] = aMoves[i];
				m[1] = bMoves[j];
				std::cout << "Next state after " << aMoves[i] << "," << bMoves[j] << " " << gdl.getNextState (initState, m) << std::endl;
			}
		}
	}
	if (roles.size() > 2){
		// GDL class suppors roles > 2; but this testcase do not
		std::cout << "Don't have debug code for role count of " << roles.size () << std::endl;
	}

	uint64_t timeEnd = microtimer ();
	std::cout << "Time needed to call calculate all this..." << (timeEnd - timeBegin) / 1000 << "ms" << std::endl;

	gdl.printDebug ();
	std::cout << "KnowledgeBase statistics: " << std::endl;
	std::cout << "Num Questions:         " << KnowledgeBase::numQuestions << std::endl;
	std::cout << "Failed Cache Hits:     " << KnowledgeBase::failedCacheHits << std::endl;
	std::cout << "Only One Cache Hits:   " << KnowledgeBase::onlyOneCacheHits << std::endl;
	std::cout << "Responder Hits: " << KnowledgeBase::responderHits << std::endl;
}
