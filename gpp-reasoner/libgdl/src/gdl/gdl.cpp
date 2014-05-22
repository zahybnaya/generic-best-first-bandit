/*
 * gdl.cpp
 *
 *  Created on: 11.11.2008
 *      Author: nos
 */
#include "gdl.h"
#include "../tools.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

GDL::State::State (){
	// rather invalid, but important for containers
}

GDL::State::State (const State & s){
	m_facts = s.getFacts ();
	m_hash  = s.getHash ();
}

GDL::State::State (const StringVec & facts){
	m_facts = facts;
	sort ();
	calcHash ();
}

GDL::State & GDL::State::operator= (const State & s){
	m_facts = s.getFacts();
	m_hash  = s.getHash ();
	return *this;
}

bool GDL::State::operator== (const State & s) const{
	if (m_hash != s.getHash()) return false;
	
	const StringVec& others = s.getFacts();
	if (others.size () != m_facts.size()) return false;
	for (size_t i = 0; i < others.size(); i++){
		if (others[i] != m_facts[i]) return false;
	}
	return true;
}

std::ostream & GDL::State::operator<< (std::ostream & ss) const {
	ss << "State: " << m_facts << " Hash=" << std::hex << m_hash << std::dec;
	return ss;
}

void GDL::State::sort (){
	std::sort (m_facts.begin(), m_facts.end());
}

void GDL::State::calcHash (){
	// very simple hash function
	unsigned int hash = 0;
	unsigned int pos  = 0;
	for (size_t i = 0; i < m_facts.size(); i++){
		const std::string & fact = m_facts[i];
		for (size_t j = 0; j < fact.length(); j++){
			hash+=fact[j] * pos ;
			pos++;
			// 31081; // a big prime
		}
	}
	m_hash = hash;
}




GDL::GDL (){
	m_initialized = false;
	m_compressing = false;
	m_optimizing = false;
}

GDL::~GDL (){
}

bool GDL::initFromFile (const std::string & filename){
	KIF kif;
	bool result = kif.parseFile (filename, false);
	if (!result) return false;
	return initFromKIF (kif);
}

bool GDL::initFromKIF (const KIF & kif){
	if (m_compressing) {
		m_compressor.init(kif, m_kb);
		std::cout << LOGID << "Using compressed KnowledgeBase" << std::endl;
	} else {
		m_kb = KnowledgeBase (kif);
	}
	
	std::string roleComp = compress ("role");
	m_roles = m_logic.simpleAsk (std::string ("(") + roleComp + " ?x)","?x", m_kb, false);
	if (m_roles.size() < 1){
		std::cerr << LOGID << "did not found any roles..";
		return false;
	}
	
	if (m_optimizing) {
		// Tuning with fake init state data
		KnowledgeBase tuned (m_kb);
		State initState = getInitState ();
		tuned.add(initState.getFacts()); // optimizer needs more material
		KBOptimizer optimizer (tuned);
		/*
		 * The optimize order system is currently not that good, so 
		 * we just fix the oder of neg/distinct as this could affect 
		 * our correctness
		 */
		// optimizer.optimizeOrder();
		optimizer.fixNegDistinctConstraint();
		optimizer.optimizeSpecialRelations();
		// Copying back the clauses (which shall be optimized now)
		// It's a bit hacky to use the non-const referenze for this ;)
		m_kb.getAllClauses() = tuned.getAllClauses();
		m_kb.getResponders () = tuned.getResponders ();
	}

	
	return true;	
}

GDL::State GDL::getInitState () const{
	std::string question = std::string ("(") + compress ("init") + " ?x)";
	std::string pattern  = std::string ("(") + compress ("true") + " ?x)";
	return m_logic.simpleAsk (question, pattern, m_kb, true);
}

const GDL::StringVec & GDL::getRoles() const{
	return m_roles;
}

unsigned int GDL::getRoleCount () const{
	return m_roles.size();
}


GDL::StringVec GDL::getPossibleMoves (size_t role, const State & state) const{
	std::string question = std::string ("(") + compress ("legal") + " " + m_roles[role] + " ?x)";
	// std::cout << LOGID << "Asking " << question << " with role=" << role << " state= " << state << std::endl;
	return m_logic.simpleAsk (question, "?x", m_kb.added (state.getFacts()), true);
}

GDL::State GDL::getNextState (const State & preState, const Move & move) const{
	assert (move.size () == m_roles.size());
	// Pushing in the move as extra information
	KnowledgeBase copy (m_kb.added(preState.getFacts()));
	for (size_t i = 0; i < move.size(); i++){
		copy.add ((compress ("(does ") + m_roles[i] + " " + move[i] + ")"));
	}
	// m_logic.setDebug();
	std::string question = std::string ("(") + compress ("next") + " ?x)";
	std::string pattern  = std::string ("(") + compress ("true") + " ?x)";
	GDL::State answer = m_logic.simpleAsk (question, pattern, copy, true);
	// m_logic.setDebug(false);
	return answer;
}

bool GDL::isFinishState (const State & state) const {
	bool result =  m_logic.isSatisfiable (compress("terminal"), m_kb.added (state.getFacts()));
	return result;
}

bool GDL::points (size_t role, const State & state, int & points){
	std::string question = std::string ("(") + compress ("goal") + " " + m_roles[role] + " ?x)";
	KnowledgeBase added = m_kb.added (state.getFacts());
	Answer * answer = m_logic.ask (question, added);
	
	bool result = answer->next ();
	if (result){
		std::string x = answer->getQuestionMap ().find ("?x")->second;
		std::string decomp = decompress (x);
		points = atoi (decomp.c_str());
	}
	delete answer;
	return result;
}

GDL::Node * GDL::getInitialNode () const{
	GDL::Node * result = new GDL::Node ();
	result->state   = getInitState ();
	if (result->state.getFacts().size() == 0){
		std::cerr << LOGID << "Error: Init node is empty" << std::endl;
		delete result;
		return 0;
	}
	result->isTerminal = isFinishState (result->state);
	result->depth = 0;
	return result;
}

bool GDL::expand (Node * node, NodeVec & target, std::vector<StringVec> & possibleMoves) const{
	assert (node);
	KnowledgeBase add (m_kb.added (node->state.getFacts()));
	for (size_t i = 0; i < m_roles.size(); i++){
		// Searching for legal moves
		std::string question = std::string ("(") + compress ("legal") + " " + m_roles[i] + " ?x)";
		StringVec p = m_logic.simpleAsk (question, "?x", add, true);

		if (p.size() == 0) {
			std::cerr << LOGID << "Could not find legal moves for role " << m_roles[i] << " in state " << node->state << " with knowledge base " << m_kb <<  std::endl;
			return false;
		}
		possibleMoves.push_back (p);
	}
	// A bit complex to generate all the combinations
	std::vector<size_t> moveDecision;
	moveDecision.resize (m_roles.size());
	for (size_t i = 0; i < moveDecision.size(); i++) moveDecision[i] = 0;
	// std::cout << std::endl << "From State " << node->state << std::endl << std::endl;
	
	NextNodeCalcList workList;
	
	while (true){
		Node * nextNode = new Node ();
		nextNode->move.resize (m_roles.size());
		
		for (size_t j = 0; j < m_roles.size(); j++) {
			nextNode->move[j] = (possibleMoves[j])[moveDecision[j]];
		}
		// std::cout << LOGID << "Calcing " << nextNode->move << std::endl;

		workList.push_back (nextNode);
		target.push_back (nextNode);

		// Going to the next one...
		int i = 0;
		for (i = m_roles.size() - 1; i >= 0; i--){
			if (moveDecision[i] == possibleMoves[i].size() - 1){
				moveDecision[i] = 0;
			} else {
				moveDecision[i]++;
				break;
			}
		}
		if (i == -1) break; // we have them all
	}
	if (!calcNextNodeWorkList(node, workList, &add)){
		deleteElements (target);
		return false;
	}
	return true;
}

bool GDL::calcNextNode (Node * baseNode, Node * nextNode, KnowledgeBase * currentKB) const {
	// Generating the knowledege for this
	KnowledgeBase addWithMove (*currentKB);
	for (size_t j = 0; j < m_roles.size(); j++) { 
		addWithMove.add (formatMove(j, nextNode->move[j]));
	}
	// we could also use KBOptimizer here; but I tried and this made everything slower
	{
		// Splitting out the new result
		std::string question = std::string ("(") + compress ("next") + " ?x)";
		std::string pattern  = std::string ("(") + compress ("true") + " ?x)";
		StringVec nextStateFacts = m_logic.simpleAsk (question, pattern, addWithMove, true);
		if (nextStateFacts.size() == 0){
			std::cerr << LOGID << "Next state is empty?! Used the following base node: " << *baseNode << " with following move " << nextNode->move << std::endl;
			return false;
		}
		nextNode->state = nextStateFacts; // does automatically convert
		nextNode->depth = baseNode->depth + 1;
		
		// is it terminal?
		nextNode->isTerminal = m_logic.isSatisfiable (compress("terminal"), m_kb.added(nextNode->state.getFacts()));
	}
	return true;
}



bool GDL::calcNextNodeWorkList (Node * baseNode, const NextNodeCalcList & workList, KnowledgeBase * currentKB) const {
	// single threading way
	for (NextNodeCalcList::const_iterator i = workList.begin(); i != workList.end(); i++){
		if (!calcNextNode (baseNode, *i, currentKB)) return false;
	}
	return true;
	
	// Thread Pool way (commented out in public version)
	// Use http://threadpool.sf.net to get this running
//	bool error = false;
//	for (NextNodeCalcList::const_iterator i = workList.begin(); i!= workList.end(); i++){
//		m_threadPool->schedule (boost::bind(&GDL::calcNextNode, (GDL*) this, baseNode, *i, currentKB, &error));
//	}
//	m_threadPool->wait();
//	return !error;
}

std::ostream & GDL::Node::operator<< (std::ostream & ss) const {
	return ss << "Node { State=" << state << " Move=" << move << " Terminal=" << isTerminal << " Depth=" << depth << "}";
}
