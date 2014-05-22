/*
 * kboptimizer.cpp
 *
 *  Created on: 14.01.2009
 *      Author: nos
 */

#include "kboptimizer.h"
#include "../tools.h"
#include <assert.h>
#include <algorithm>
#include <deque>
#include "logic.h"

const VariableMap NoAnswer::emptyQMap;

LinearResponder::LinearResponder(const RelationSignature & signature, const StringDeque & chain){
	m_sig = signature;
	m_chain = chain;
}

bool LinearResponder::applicable (const std::string & question) const {
	// Linear responder answers questions with 0 or 1 variable
	// we already assume that the signature fits, as KnowledgeBase checks this
	return KIF::getVariables(question).size() < 2;
}

Answer * LinearResponder::ask (const std::string & question, unsigned int depth) const {
	// split out the two variables..
	int size = question.size();
	STACKSTRING (question_stack, qs, size);
	qs.setTo (question);
	
	STACKSTRING (name_stack, ns, size);
	STACKSTRING (arg1_stack, a1s, size);
	STACKSTRING (arg2_stack, a2s, size);
	
	STACKSTRING (tail_stack, ts, size);
	KIF::splitHeadTail (qs, ns, ts);
	if (ns != m_sig.name){
		std::cerr << LOGID << "Question has wrong signature, why not checked, question: " << question << " my signature " << m_sig.name << " ?" << std::endl;
		return 0;
	}
	KIF::splitHeadTail (ts, a1s, ts);
	KIF::splitHeadTail (ts, a2s, ts);
	bool a1v = KIF::isVariable (a1s);
	bool a2v = KIF::isVariable (a2s);
	assert (! (a1v && a2v)); // must be checked before by applicable
	// search the second in the chain
	StringDeque::const_iterator i = m_chain.begin();
	StringDeque::const_iterator j = i;
	i++;
	for (;i!=m_chain.end(); i++){
		if (a1v && (a2s == *i)){ // found it
			VariableMap qmap;
			qmap[(std::string) a1s] = *j;
			return new SingleAnswer (qmap, question);
		}
		if (a2v && (a1s == *j)){ // found it
			VariableMap qmap;
			qmap[(std::string) a2s] = *i;
			return new SingleAnswer (qmap, question);
		}
		if (!a1v && !a2v && (a1s == *j) && (a2s == *i)){ // found it
			return new SingleAnswer (VariableMap(), question);
		}
		j++;
	}	
	// did not found it
	return new NoAnswer (question);
}

ComparisonResponder::ComparisonResponder (const RelationSignature & sig, const StringDeque & chain, bool isSmaller){
	m_sig = sig;
	m_chain = chain;
	if (!isSmaller) std::reverse (m_chain.begin(), m_chain.end());
}

bool ComparisonResponder::applicable (const std::string & question) const {
	// Comparisonresponder only feels applicable if there are no variables
	return !KIF::hasVariables (question);
}

Answer * ComparisonResponder::ask(const std::string & question, unsigned int depth) const {
	/*
	 * We assume that the signature fits, and that there are no variables
	 * (comparison arg1 arg1)
	 */
	int size = question.size();
	STACKSTRING (head_stack, hs, size);
	STACKSTRING (tail_stack, ts, size);
	ts.setTo (question);
	KIF::splitHeadTail (ts, hs, ts);
	assert (hs == m_sig.name);
	KIF::splitHeadTail (ts, hs, ts);
	std::string arg1 (hs);
	KIF::splitHeadTail (ts, hs, ts);
	std::string arg2 (hs);
	bool i1Found = false;
	bool i2Found = false;
	for (StringDeque::const_iterator i = m_chain.begin(); i != m_chain.end(); i++){
		if (*i == arg1)
			i1Found = true;
		else 
			if (*i == arg2) {
				i2Found = true;
				break;
			}
	}
	if (i1Found && i2Found){
		return new SingleAnswer (VariableMap (), question);
	}
	return new NoAnswer (question);	
}

KBOptimizer::~KBOptimizer() {
	// TODO Auto-generated destructor stub
}

void KBOptimizer::optimizeOrder (){
#ifndef NDEBUG
	uint64_t start = microtimer ();
#endif
	RelationMap relationMap;
	exportRelations (relationMap);
	calcImpact (relationMap);
	// std::cout << LOGID << "TODO: Currently doing nothing, just print out." << std::endl;
	// std::cout << LOGID << relationMap << std::endl;
	ClauseMap & clauses = m_kb.getAllClauses();
	for (ClauseMap::iterator i = clauses.begin(); i!= clauses.end(); i++){
		for (ClauseVec::iterator j = i->second.begin(); j!=i->second.end(); j++){
			Clause & clause = *j;
			clause.calcImpact(relationMap);
			clause.reorder();
		}
	}
#ifndef NDEBUG
	uint64_t end = microtimer ();
	std::cout << LOGIDXL << "Time to do full reordering:  = " << end - start << " µs." << std::endl;
#endif
}

void KBOptimizer::fixNegDistinctConstraint (){
	ClauseMap & clauses = m_kb.getAllClauses();
	for (ClauseMap::iterator i = clauses.begin(); i!= clauses.end(); i++){
		for (ClauseVec::iterator j = i->second.begin(); j!=i->second.end(); j++){
			Clause & clause = *j;
			clause.fixNegDistinctConstraint();
		}
	}
}

void KBOptimizer::exportRelations(RelationMap & relationMap) const {
	/// going through the facts
	const FactMap & facts = m_kb.getAllFacts();
	for (FactMap::const_iterator i = facts.begin(); i!= facts.end(); i++){
		const RelationSignature & sig = i->first;
		for (FactVec::const_iterator j = i->second.begin(); j!= i->second.end(); j++){
			const Fact & fact = *j;
			RelationInfo & info = relationMap.findOrAdd (sig);
			bool hv = KIF::hasVariables (fact.text);
			info.factCount += 1;
			info.factWithVariables = info.factWithVariables || hv;
		}
	}
	/// going through the clauses
	const ClauseMap & clauses = m_kb.getAllClauses();
	for (ClauseMap::const_iterator i = clauses.begin(); i!= clauses.end(); i++){
		const RelationSignature & sig = i->first;
		for (ClauseVec::const_iterator j = i->second.begin(); j!= i->second.end(); j++){
			const Clause & clause = *j;
			RelationInfo & info = relationMap.findOrAdd (sig);
			info.clauseCount++;
			RelationDependency dependency;
			for (Clause::PremissList::const_iterator k = clause.premisses.begin(); k!= clause.premisses.end(); k++){
				const Premiss & p = *k;
				RelationSignature psig (p.text);
				dependency.insert(psig);
			}
			info.dependencies.push_back(dependency);
		}
	}
	
	// Impact for fact-only functions is easy to calculate
	for (RelationMap::iterator i = relationMap.begin(); i!= relationMap.end(); i++){
		RelationInfo & info = i->second;
		if (info.clauseCount == 0){
			info.impact = info.factCount;
		}
	}
	
	// TODO
	// Calculating
	// * detecting transitive functions
	// * detecting comparison on transitive functions
}

void KBOptimizer::calcImpact(RelationMap & relationMap) {
	// going through the map and impact will flow from the facts to the clauses, if there are no 
	// circles
	int iterationCount = 0;
	bool changes;
	do {
		changes = false;
		for (RelationMap::iterator i = relationMap.begin(); i!= relationMap.end(); i++){
			RelationInfo & info = i->second;
			if (info.clauseCount == 0) continue;
			int before = info.impact;
			info.impact = 1;
			
			for (RelationDependencies::iterator j = info.dependencies.begin(); j!=info.dependencies.end(); j++){
				int depImpact = 1;
				for (RelationDependency::iterator k = j->begin(); k!= j->end(); k++){
					RelationMap::iterator l = relationMap.find (*k);
					if (l == relationMap.end()) continue;
					RelationInfo & d = l->second;
					depImpact *= d.impact;
				}
				info.impact+=depImpact;
				if (info.impact > MAX_IMPACT) info.impact = MAX_IMPACT;
			}
			
			info.impact += info.factCount;
			if (info.impact > MAX_IMPACT) info.impact = MAX_IMPACT;
			if (info.impact != before) {
				changes = true;
			}
		}
		iterationCount++;
	} while (changes && iterationCount < 32);
	if (iterationCount == 32){
		std::cerr << LOGID << "Max iteration count in impact calculation reached!!" << std::endl;
	}
#ifndef NDEBUG
	std::cout << LOGID << "Iteration Count= " << iterationCount << std::endl;
#endif
}

typedef std::map<std::string, std::string> SSMap; // String-String-Map

void KBOptimizer::optimizeSpecialRelations(){
#ifndef NDEBUG
	uint64_t start = microtimer ();
#endif
	RelationMap relationMap;
	exportRelations (relationMap);
	findLinearRelations (relationMap);
	findComparisonRelations (relationMap);
	
	// Adding special optimizers...
	int responderCount = 0;
	for (RelationMap::const_iterator i = relationMap.begin(); i!= relationMap.end(); i++){
		const RelationInfo & info = i->second;
		const RelationSignature & sig = i->first;
		if (info.specialType & RelationInfo::LINEAR){
			ResponderPtr responder (new LinearResponder (sig, info.linearChain));
			if (m_kb.addResponder (responder)) responderCount++;
		}
		if (info.specialType & RelationInfo::SMALLER){
			ResponderPtr responder (new ComparisonResponder (sig, info.linearChain, /*isSmaller*/ true));
			if (m_kb.addResponder (responder)) responderCount++;
		}
		if (info.specialType & RelationInfo::GREATER){
			ResponderPtr responder (new ComparisonResponder (sig, info.linearChain, /*isSmaller*/ false));
			if (m_kb.addResponder (responder)) responderCount++;
		}
	}
	
#ifndef NDEBUG
	uint64_t end = microtimer ();
	std::cout << LOGIDXL << "Time to optimize special relations:  = " << end - start << " µs." << std::endl;
	std::cout << LOGID << "Responders added: " << responderCount << std::endl;
#endif
}

void KBOptimizer::findLinearRelations (RelationMap & relationMap){
#ifndef NDEBUG
	uint64_t start = microtimer ();
#endif
	// linear functions have to be /2, fact only and no variables, and more than 2 facts (less makes no sense)
	std::cout << LOGID << "(Debug .. searching Linear Relations)" << std::endl;
	for (RelationMap::iterator i = relationMap.begin(); i!= relationMap.end(); i++){
		const RelationSignature & sig = i->first;
		RelationInfo & info = i->second;
		if (sig.argc == 2 && info.clauseCount == 0 && info.factWithVariables == false && info.factCount > 2){
			std::cout << LOGID << "Investigating candidate " << sig << std::endl;
			const FactVec & facts = m_kb.getFacts(sig);
			std::set<std::string> symbols; // all symbols in relation
			SSMap connections; // the connections in relation
			SSMap  backConnections; // inverse relation
			bool failed = false;
			for (FactVec::const_iterator j = facts.begin(); j != facts.end(); j++){
				const Fact & f = *j;
				// fact of the type (rel x y)
				symbols.insert (f.args[0]);
				symbols.insert (f.args[1]);
				if (connections.find (f.args[0]) != connections.end()){
					// x points already to something
					failed = true;
					break;
				}
				if (backConnections.find (f.args[1]) != backConnections.end()){
					// something already points to y
					failed = true;
					break;
				}
				connections[f.args[0]] = f.args[1];
				backConnections[f.args[1]] = f.args[0];
			}
			if (failed) continue;
			
			// now checking that they create a chain
			StringDeque chain;
			// putting in the start element
			chain.push_back (*symbols.begin());
			bool change;
			do {
				change = false;
				{	// trying to complete it at the beginning...
					const std::string & first = chain.front();
					SSMap::const_iterator k = backConnections.find (first);
					if (k != backConnections.end()){
						chain.push_front(k->second);
						change = true;
					}
				}
				{	// trying to complete it at the end...
					const std::string & last = chain.back();
					SSMap::const_iterator k = connections.find (last);
					if (k != connections.end()){
						chain.push_back (k->second);
						change = true;
					}
				}
			} while (change && chain.size() <= symbols.size());
			if (chain.size() != symbols.size()){
				std::cout << LOGID << "Could not put everything into the chain or there is a loop" << std::endl;
				std::cout << "Current Chain: " << chain << std::endl;
				std::cout << "Symbols:       " << symbols << std::endl;
				// std::cout << "Facts:         " << facts << std::endl;
				continue;
			}
			std::cout << LOGID << "Identified a linear function: " << sig << std::endl;
			std::cout << LOGID << "Symbol Chain: " << chain << std::endl;
			// std::cout << LOGID << "Frontconnections: " << connections << std::endl;
			// std::cout << LOGID << "BackConnections:  " << backConnections << std::endl;
			info.specialType = RelationInfo::LINEAR;
			info.linearChain = chain;
		}
		
	}
#ifndef NDEBUG
	uint64_t end = microtimer ();
	std::cout << LOGIDXL << "Time to find linears:  = " << end - start << " µs." << std::endl;
#endif
}

void KBOptimizer::findComparisonRelations (RelationMap & relationMap){
#ifndef NDEBUG
	uint64_t start = microtimer ();
#endif
	// std::cout << LOGID << "(Debug .. searching Linear Relations)" << std::endl;
	Logic logic; // we need it for unifiying
	for (RelationMap::iterator i = relationMap.begin(); i!= relationMap.end(); i++){
		const RelationSignature & sig = i->first;
		RelationInfo & info = i->second;
		if (info.specialType) {
			// std::cout << "Skipping " << sig << " as it is already marked as being special" << std::endl;
			continue;
		}
		// std::cout << LOGID << "Looking at " << sig << " info: "<< info << std::endl;
		if (sig.argc != 2) {
			// std::cout << "Dropping - wrong count" << std::endl;
			continue;
		}
		// Dependency to itself and linear only
		
		RelationSignature linear;
		bool foundLinear = false;
		bool valid = true;
		for (RelationDependencies::const_iterator j = info.dependencies.begin(); j != info.dependencies.end() && valid; j++){
			for (RelationDependency::const_iterator k = j->begin(); k!=j->end(); k++){
				const RelationSignature & dsig = *k;
				if (dsig == sig) continue;
				if (dsig.name == "distinct" && dsig.argc == 2){
					// std::cout << LOGID << "Ignoring distinct, this is not good behaviour, but gets checked later" << std::endl;
					continue;
				}
				if (!foundLinear){
					// checking whether dsig is linear
					RelationMap::const_iterator l = relationMap.find (dsig);
					if ((l == relationMap.end()) || (l->second.specialType != RelationInfo::LINEAR)){
						// invalid dependency
						// std::cout << "Dropping " << sig << " because of invalid dependencies" << std::endl;
						valid = false;
						break;
					}
					foundLinear = true;
					linear = dsig;					
				} else {
					if (dsig != linear) {
						// std::cout << "Dropping " << sig << " because of being dependent from more than one linear" << std::endl;
						valid = false;
						break;
					}
				}
			}
			if (!valid) break;
		}
		if (!valid) continue; 
		if (!foundLinear) {
			// std::cout << "Dropping " << sig << " because of not being dependet on a linear" << std::endl;
			continue;
		}
		
		// std::cout << "Comparing comparison with handmade results" << std::endl;
		
		StringVec regular;
		// artificial smaller than operation
		regular.push_back ("(<= (_smaller ?y ?x) (" + linear.name + " ?y ?x))"); // y < x
		regular.push_back ("(<= (_smaller ?y ?x) (" + linear.name + " ?z ?x) (_smaller ?y ?z))"); // y < z < x
		// artificial greater than operation
		regular.push_back ("(<= (_greater ?y ?x) (" + linear.name + " ?x ?y))"); // y > x
		regular.push_back ("(<= (_greater ?y ?x) (" + linear.name + " ?z ?y) (_greater ?z ?x))"); // y > z > x
		KnowledgeBase extended = m_kb.added(regular);
		Logic logic;
		
		const RelationInfo & linearInfo = relationMap[linear];
		
		if (linearInfo.linearChain.size() > 100){
			std::cout << LOGID << "Skipping test of " << sig << " as it's dependent is too big" << std::endl;
			std::cout << LOGID << "(nosc: please fix that)" << std::endl;
			continue;
		}
		StringVec candidate = logic.simpleAsk ("(" + sig.name + " ?x ?y)", "(?x ?y)", m_kb, true);
		StringVec smallerResult = logic.simpleAsk ("(_smaller ?x ?y)", "(?x ?y)", extended, true);
		if (candidate == smallerResult){
			std::cout << LOGID << sig << " is smallerThan " << std::endl;
			info.specialType = RelationInfo::SMALLER;
			info.linearChain = linearInfo.linearChain;
			continue;
		}
		StringVec greaterResult = logic.simpleAsk ("(_greater ?x ?y)", "(?x ?y)", extended, true);
		if (candidate == greaterResult) {
			std::cout << LOGID << sig << " is greaterThan " << std::endl;
			info.specialType = RelationInfo::GREATER;
			info.linearChain = linearInfo.linearChain;
			continue;
		}
		// std::cout << LOGID << "Had no luck with " << sig << std::endl;
	}
#ifndef NDEBUG
	uint64_t end = microtimer ();
	std::cout << LOGIDXL << "Time to find comparisons:  = " << end - start << " µs." << std::endl;
#endif
}
