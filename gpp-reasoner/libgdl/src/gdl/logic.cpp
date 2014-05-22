/*
 * logic.cpp
 *
 *  Created on: 12.11.2008
 *      Author: nos
 */

#include "logic.h"
#include "../logid.h"
#include "../tools.h"
#include <assert.h>
#include "../faststringcollector.h"

bool Logic::AnswerDecoder::next() {
	bool result = m_answer->next();
	if (result) {
		m_questionMap = decodeAllVariables (m_answer->getQuestionMap());
	}
	if (result) m_wasTrueTimes++;
	if (!result){
		if (m_wasTrueTimes == 0){
			// we can give a hint to the knowledge base that we found a unforfillable question
			m_kb.failedHint (m_originalQuestion);
		}
		if (m_wasTrueTimes == 1){
			// we can give a hint to the knowledge base that we found a question with only one solution
			m_kb.onlyOneHint (m_originalQuestion, m_questionMap);
		}
	}
	
	return result;
}


Logic::ClauseAnswer::ClauseAnswer (const Logic & l, const RelationSignature & sig, const std::string & question, const KnowledgeBase & kb, unsigned int depth)
: m_logic (l), m_kb (kb), m_question (question), m_sig (sig){
	m_position = 0;
	m_depth = depth;
	m_onAnAnswer = false;
}


Logic::ClauseAnswer::~ClauseAnswer (){
	// if stack is yet net empty we have to delete some subresults
	for (std::deque<SubAnswer>::iterator i = m_subAnswers.begin(); i!=m_subAnswers.end(); i++){
		delete i->partAnswer;
	}
}

bool Logic::ClauseAnswer::next (){
	m_onAnAnswer = true;

	const KnowledgeBase::FactVec & facts = m_kb.getFacts(m_sig);
	const KnowledgeBase::ClauseVec & clauses = m_kb.getClauses(m_sig);
	
	if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth (m_depth) << "Checking question " << m_question << " with signature " << m_sig << " against " << facts.size() << " facts and " << clauses.size() << " clauses" << std::endl;
	
	if (m_position >= facts.size() + clauses.size()){
		m_onAnAnswer = false;
		return false;
	}

	for (;m_position < facts.size(); m_position++){
		std::string fact = facts[m_position].text;
		if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth (m_depth) << "Checking against fact " << fact << std::endl;
		m_questionMap = VariableMap ();
		if (m_logic.unify_mgu_compressed(fact, m_question, m_questionMap)){
			m_position++;
			m_isFactAnswer = true;
			if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth (m_depth) << "Yes, fact " << fact << " unifies with " << m_questionMap << std::endl;
			return true;
		}
	}
	while (m_position < facts.size() + clauses.size()){
		const Clause & clause = clauses[m_position - facts.size()]; // the clause against we find solutions
		if (m_subAnswers.empty()){
			// putting it on the stack
			VariableMap headMap; // needs to be clear on each iteration
			// checking the head of the clause
			if (m_logic.unify_mgu_compressed (clause.head, m_question, headMap, m_depth)){
				if (clause.premisses.empty()){
					// clauses having no tail
					// This is very seldom; but possible, that there is no tail in the clause
					// e.g. in example maze.kif
					m_position++;
					m_questionMap = headMap;
					return true;
				}
				// putting sub answers onto the stack
				SubAnswer sanswer;
				sanswer.nextPremiss = clause.premisses.begin();
				sanswer.headMap = headMap;
				Clause::PremissList::const_iterator & current = sanswer.nextPremiss;
				sanswer.partAnswer = m_logic.ask (Logic::applyVariableMap(headMap, current->text), m_kb, m_depth + 1);
				// nextPremiss always shows to the next one
				current++; // as it is a reference, this auto updates sanswer.nextPremiss
				m_subAnswers.push_back(sanswer);
			}
		}
		while (!m_subAnswers.empty()){
			SubAnswer & tail = m_subAnswers.back();
			if (tail.partAnswer->next()){
				// Ok lets go further
				if (tail.nextPremiss != clause.premisses.end()){
					SubAnswer nextTail;
					nextTail.nextPremiss = tail.nextPremiss;
					nextTail.nextPremiss++; // shall show onto the next
					nextTail.headMap    = compressVariableMap (combine (tail.headMap, tail.partAnswer->getQuestionMap()));
					// the following is not enough and cannot replace compressVariabelMap (combine (..))
					// nextTail.headMap    = applyVariableMap (tail.partAnswer->getQuestionMap(), tail.headMap);
					const Premiss & current = * tail.nextPremiss;
					nextTail.partAnswer = m_logic.ask (applyVariableMap (nextTail.headMap, current.text), m_kb, m_depth + 1);
					m_subAnswers.push_back (nextTail);
				} else {
					// hey, this was a final solution
					if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth(m_depth) << "[D: " << m_depth << "] Having a result ";

					m_questionMap = compressVariableMap (combine (tail.headMap, tail.partAnswer->getQuestionMap()));
					// the following is not enough and cannot replace compressVariabelMap (combine (..))
					// m_questionMap = applyVariableMap (tail.partAnswer->getQuestionMap(), tail.headMap); // faster?
					if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth (m_depth) << " with QuestionMap of " << m_questionMap << std::endl;
					return true;
				}
			} else {
				delete tail.partAnswer;
				m_subAnswers.pop_back ();
			}
		}
		m_position++;
	}
	if (m_logic.getDebugLevel() > 0) std::cout << LOGID << formatDepth (m_depth) << "D=" << m_depth << " no further solution for " << m_question << std::endl;
	m_onAnAnswer = false;
	return false;
}

std::ostream & Logic::ClauseAnswer::backtrace (std::ostream & bt, unsigned int depth) const {
	const KnowledgeBase::FactVec & facts   = m_kb.getFacts   (m_sig);
	const KnowledgeBase::ClauseVec & clauses = m_kb.getClauses (m_sig);

	// Against Facts
	bt << depth << ":";
	if (m_isFactAnswer){
		// HACK m_position already looks on the next one; this is stupid behaviour
		unsigned int pos = m_position - 1;
		if (pos < facts.size()){
			const Fact & fact = facts[pos];
			return bt << m_question << " on fact " << fact << " with " << m_questionMap << std::endl;
		}
	}
	// Against Clauses
	const Clause & clause = clauses[m_position - facts.size()];
	bt << m_question << " on " << clause << "  " << std::endl;
	for (std::deque<SubAnswer>::const_iterator i = m_subAnswers.begin(); i!=m_subAnswers.end(); i++){
		i->partAnswer->backtrace(bt, depth + 1);
	}
	return bt;
}

Logic::OrClauseAnswer::OrClauseAnswer
(const Logic & l, const std::string & question, const KnowledgeBase & kb, unsigned int depth) :
	m_logic (l), m_kb (kb) {
	m_tokenizer  = KIF::Tokenizer (KIF::stripBraces(question));
	std::string token;
	m_tokenizer.next (token);
	assert (token == "or");
	m_currentAnswer = 0;
	m_depth = depth;
}

Logic::OrClauseAnswer::~OrClauseAnswer () {
}

bool Logic::OrClauseAnswer::next (){
	if (m_currentAnswer){
		if (m_currentAnswer->next()){
			m_questionMap = m_currentAnswer->getQuestionMap ();
			return true;
		}
	}
	// go to the next one
	std::string token;

	while (m_tokenizer.next(token)) {
		delete m_currentAnswer;
		m_currentAnswer = m_logic.ask(token, m_kb, m_depth + 1);
		if (m_currentAnswer->next()){
			m_questionMap = m_currentAnswer->getQuestionMap ();
			return true;
		}
	}
	delete m_currentAnswer;
	m_currentAnswer = 0;
	return false;
}

std::ostream & Logic::OrClauseAnswer::backtrace (std::ostream & bt, unsigned int depth) const {
	if (m_currentAnswer){
		bt << depth << m_tokenizer.getString () << " at " << std::endl;
		m_currentAnswer->backtrace (bt, depth + 1);
	}
	return bt;
}


Logic::DistinctAnswer::DistinctAnswer (const Logic & l, const std::string & q, const KnowledgeBase & kb, unsigned int depth) : m_logic (l){
	m_returnedResult = false;
	m_tokenizer = KIF::Tokenizer (KIF::stripBraces(q));
	std::string token0, token1, token2;
	m_tokenizer.next (token0);
	m_tokenizer.next (token1);
	m_tokenizer.next (token2);
	assert (!m_tokenizer.hasNext ());
	assert (token0 == "distinct");
	m_distinct = (token1 != token2);
}

bool Logic::DistinctAnswer::next (){
	if (m_returnedResult) return false;
	m_returnedResult = true;
	return m_distinct;
}

std::ostream & Logic::DistinctAnswer::backtrace (std::ostream & bt, unsigned int depth) const {
	if (m_returnedResult){
		bt << depth << ":" << m_tokenizer.getString() << " is true";
	}
	return bt;
}


Logic::NotAnswer::NotAnswer (const Logic & l, const std::string & q, const KnowledgeBase & kb, unsigned int depth)  :  m_logic (l), m_question (q) {
	m_questionMap = VariableMap ();
	KIF::Tokenizer tokenizer (KIF::stripBraces (q));
	std::string token0, token1;
	tokenizer.next (token0);
	tokenizer.next (token1);
	if (tokenizer.hasNext()){
		std::string token2;
		tokenizer.next (token2);
		std::cerr << LOGID << "Bad 'not' Syntax " << token2 << " in " << q << std::endl;
		m_not = false;
		m_returnedResult = true;
		return;
	}
	assert (token0 == "not");
	m_subAnswer = m_logic.ask (token1, kb, depth + 1);
	m_returnedResult = false;
}

Logic::NotAnswer::~NotAnswer (){
	delete m_subAnswer;
}

bool Logic::NotAnswer::next (){
	if (m_returnedResult) return false;

	m_not = !m_subAnswer->next();
	m_returnedResult = true;
	return m_not;
}

std::ostream & Logic::NotAnswer::backtrace (std::ostream & bt, unsigned int depth) const {
	if (m_not){
		bt << depth << ":" << m_question << " could not be satisfied, so returning true" << std::endl;
	}
	return bt;
}

Logic::Logic (){
	m_debugLevel = 0;
	m_maxUniformDepth = 512;
	m_maxQueryDepth   = 128;
	m_doOccureCheck = true;
}

Logic::~Logic (){
}

Answer * Logic::ask  (const std::string & question, const KnowledgeBase & kb, unsigned int depth) const {
#ifndef NDEBUG
	if (m_calls.find (question) == m_calls.end()) m_calls[question] = 1;
	else m_calls[question]++;
#endif
	if (m_debugLevel > 0) std::cout << LOGID << "Answering " << question << " at depth= " << depth << std::endl;
	RelationSignature sig (question);
	
	// At first we wcheck out whether KnowledgeBase has an optimized version for us:
	Answer * optimized = kb.canRespond (sig, question);
	if (optimized){	
//		/*
//		 * Self Check
//		 */
//		if (sig.name != "not" && sig.name != "distinct" && sig.name != "or"){
//			Answer * first  = kb.canRespond (sig, question);
//			assert (first);
//			std::string eQ = AnswerDecoder::getEncodedQuestion(question, depth);
//			Answer * second = new AnswerDecoder (new ClauseAnswer (*this, sig, eQ, kb, depth), question, kb);
//			
//			bool error = false;
//			while (first->next()){
//				bool sNext = second->next();
//				if (!sNext){
//					std::cerr << LOGID << "reference has less results, question=" << question << std::endl;
//					std::cerr << LOGID;
//					first->backtrace(std::cerr, depth);
//					error = true;
//					break;
//				}
//				if (first->getQuestionMap() != second->getQuestionMap()){
//					std::cerr << LOGID << "QuestionMap mismatch" << std::endl;
//					std::cerr << LOGID << "First: " << first->getQuestionMap() << " Second: " << second->getQuestionMap() << std::endl;
//					error = true;
//					break;
//				}
//			}
//			bool sNext = second->next();
//			if (sNext) {
//				std::cerr << LOGID << "Second has more results: " << second->getQuestionMap() << std::endl;
//				error = true;
//			}
//			if (error) std::cerr << LOGID << "Had an error in question=" << question << std::endl;
//			delete first;
//			delete second;
//			
//		}
		return optimized;
	}

	std::string encodedQuestion = AnswerDecoder::getEncodedQuestion(question, depth);
	
	const std::string & command = sig.name;
	if (m_debugLevel > 0) std::cout << LOGID << "\tEncoded into " << encodedQuestion << " with signature " << sig << std::endl;
	/*
	 * THIS IS THE PLACE WHERE WE COULD INSERT MORE OPTIMIZED QUERYS
	 * AND PREIMPLEMENTED TOKENS (TODO)
	 */
	if (command == "or"){
		return new AnswerDecoder (new OrClauseAnswer (*this, encodedQuestion, kb, depth), question, kb);
	}
	if (command == "distinct"){
		return new AnswerDecoder (new DistinctAnswer (*this, encodedQuestion, kb, depth), question, kb);
	}
	if (command == "not"){
		return new AnswerDecoder (new NotAnswer (*this, encodedQuestion, kb, depth), question, kb);
	}
	
	return new AnswerDecoder (new ClauseAnswer (*this, sig, encodedQuestion, kb, depth), question, kb);
}

Logic::StringVec Logic::simpleAsk (const std::string & question, const std::string & pattern, const KnowledgeBase & kb, bool checkForDoubles) const {
	StringVec result;
	Answer * answer = ask(question, kb);
	if (!checkForDoubles){
		while (answer->next()){
			const VariableMap & map = answer->getQuestionMap();
			result.push_back (applyVariableMap (map, pattern));
		}
	}
	else {
		std::set<std::string> already;
		while (answer->next()){
			const VariableMap & map = answer->getQuestionMap ();
			std::string v = applyVariableMap (map, pattern);
			if (already.find(v) == already.end()){
				already.insert(v);
				// result.push_back (v);
			}
		}
		// copying results, sorted
		for (std::set<std::string>::const_iterator i = already.begin(); i!= already.end(); i++){
			result.push_back (*i);
		}
	}
	delete answer;
	return result;
}

bool Logic::isSatisfiable(const std::string & question, const KnowledgeBase & kb) const {
	Answer * answer = ask(question, kb);
	bool result = answer->next ();
	delete answer;
	return result;
}

bool Logic::unify_mgu_compressed (const std::string & x, const std::string & y, VariableMap & mgu, unsigned int depth) const {
	// putting data into faster SStrings
	STACKSTRING (xs_stack, xs, x.length());
	xs.setTo (x);
	STACKSTRING (ys_stack, ys, y.length());
	ys.setTo (y);
	bool result = unify_mgu (xs, ys, mgu, depth);
	if (result){
		mgu = compressVariableMap (mgu);
	}
	return result;
}

bool Logic::unify_mgu (const SString & x, const SString & y, VariableMap & mgu, unsigned int depth) const {
	// Like in the Artificial Intelligence: A modern approach Book
	if (m_debugLevel > 1) std::cout << LOGID << "D=" << depth << "Checking " << x << " vs " << y << " with mgu=" << mgu << " ";
	if (depth > m_maxUniformDepth) {
		std::cerr << LOGID << "Max uniform depth " << m_maxUniformDepth << " reached" << std::endl;
		return false;
	}
	if (x == y) {
		if (m_debugLevel > 1) std::cout << "[Ok, Same]" << std::endl;
		return true;
	}
	if (KIF::isVariable(x)){
		if (m_debugLevel > 1) std::cout << std::endl;
		return unify_mguvar (x,y,mgu, depth + 1);
	}
	if (KIF::isVariable(y)){
		if (m_debugLevel > 1) std::cout << std::endl;
		return unify_mguvar (y,x,mgu, depth + 1);
	}
	if (!KIF::isBraced(x)){
		if (KIF::isBraced(y) || x!=y) {
			if (m_debugLevel > 1) std::cout << "[F, Different]" << std::endl;
			return false; 
		} else { 
			// According to book this can happen, 
			// but i see only the case that y is not braced AND x==y which is already done above
			assert (false && "How can this happen?");
			if (m_debugLevel > 1) std::cout << "[Ok2]" << std::endl;
			return true;
		}
	}
	if (!KIF::isBraced(y)){
		if (m_debugLevel > 1) std::cout << "[F, Different bracing]" << std::endl;
		return false;
	}
	if (m_debugLevel > 1) std::cout << "[Down]" << std::endl;
	STACKSTRING (xhead_stack, xhead, x.length());
	STACKSTRING (xtail_stack, xtail, x.length());
	STACKSTRING (yhead_stack, yhead, y.length());
	STACKSTRING (ytail_stack, ytail, y.length());
	KIF::splitHeadTail (x, xhead, xtail);
	KIF::splitHeadTail (y, yhead, ytail);
	if (!unify_mgu (xhead, yhead, mgu, depth + 1)) return false;
	return unify_mgu (xtail, ytail, mgu, depth + 1);
}

bool Logic::unify_mguvar (const SString & var, const SString & value, VariableMap & mgu, unsigned int depth) const {
	if (depth > m_maxUniformDepth) {
		std::cerr << LOGID << "Max uniform depth " << m_maxUniformDepth << " reached" << std::endl;
		return false;
	}
	// Like in the Artificial Intelligence: A modern approach Book
	VariableMap::iterator i = mgu.find(var);
	if (m_debugLevel > 1) std::cout << LOGID << "Checking " << var << " to " << value << ":";
	if (i != mgu.end()){
		if (m_debugLevel > 1) std::cout << "[already, checking unify]" << std::endl;
		STACKSTRING (var_stack, var, i->second.length());
		var.setTo (i->second);
		return unify_mgu (/*i->second*/ var, value, mgu, depth + 1);
	} else {
		if (m_doOccureCheck && unify_mgucheck (var,value,mgu, depth + 1)){
				if (m_debugLevel > 1) std::cout << " [occuring, failed]" << std::endl;
				return false;
		} 
		if (m_debugLevel > 1) std::cout << "[Ok]" << var << "-->" << value << std::endl;
		mgu[var] = value;
		return true;
	}
}

bool Logic::unify_mgucheck (const SString & var, const SString & value, VariableMap & mgu, unsigned int depth) const {
	if (depth > m_maxUniformDepth) {
		std::cerr << LOGID << "Max uniform depth " << m_maxUniformDepth << " reached" << std::endl;
		return true; // returning true as it breaks the function above and we want to stop the process..
	}
	// Like in the Artificial Intelligence: A modern approach Book
	if (var == value) return true;
	if (KIF::isVariable(value)){
		VariableMap::iterator i = mgu.find (value);
		if (i != mgu.end()){
			STACKSTRING (replace_stack, replace, i->second.length());
			replace.setTo (i->second);
			return unify_mgucheck (var, /*i->second*/ replace, mgu, depth + 1);
		}
		return false;
	}
	if (KIF::isNotBraced (value) || value == "()"){
		return false;
	}
	STACKSTRING (vhead_stack, vhead, value.length());
	STACKSTRING (vtail_stack, vtail, value.length());
	KIF::splitHeadTail (value, vhead, vtail);
	if (unify_mgucheck (value, vhead, mgu, depth + 1)) return true;
	else return unify_mgucheck (value, vtail, mgu, depth + 1);
}


std::string Logic::applyVariableMap (const VariableMap & map, const std::string & base){
//	// regular version
//	KIF::SyntacticalTokenizer tokenizer (base);
//	std::ostringstream output;
//	std::string token;
//	while (tokenizer.next (token)){
//		VariableMap::const_iterator i;
//		if ((i = map.find (token)) != map.end()){
//			output << i->second;
//		} else {
//			output << token;
//		}
//	}
//	return output.str();

	// faster reimplementation using stackstring
	if (map.empty()) return base;
	// std::ostringstream xoutput;
	FastStringCollector xoutput;
	int len = base.length();
	STACKSTRING (base_stack, bs, len);
	STACKSTRING (head_stack, hs, len);
	int changes = 0;
	bs.setTo(base);
	do {
		KIF::syntacticalSplitHeadTail(bs,hs,bs);
		VariableMap::const_iterator i;
		if (KIF::isVariable(hs) && (( i = map.find (hs)) != map.end())){
			xoutput << i->second;
			changes++;
		} else {
			xoutput << hs;
		}
		
	} while (bs.length() > 0);
	if (changes == 0) return base;
	return xoutput.str();
}

VariableMap Logic::applyVariableMap (const VariableMap & map, const VariableMap & base){
	if (map.empty()) return base;
	VariableMap output = base;
	for (VariableMap::iterator i = output.begin(); i!= output.end(); i++){
		i->second = applyVariableMap (map, i->second);
	}
	// std::cout << "Applying " << map << " to " << base << " coming to " << std::endl << "\t" << output << std::endl;
	return output;
}

std::string applyVariableMap (const std::string & var, const std::string & to, const std::string & base){
	// faster reimplementation
	FastStringCollector output;
	int len = base.length();
	STACKSTRING (head_stack, hs, len);
	STACKSTRING (tail_stack, ts, len);
	ts.setTo (base);
	do {
		KIF::syntacticalSplitHeadTail(ts, hs, ts);
		if (hs == var){
			output << to;
		} else {
			output << hs;
		}
	} while (ts.length() > 0);
	return output.str();
//	KIF::SyntacticalTokenizer tokenizer (base);
//	std::ostringstream output;
//	std::string token;
//	while (tokenizer.next (token)){
//		if (token == var) output << to;
//		else output << token;
//	}
//	return output.str();
}

VariableMap Logic::compressVariableMap (const VariableMap & map){
	VariableMap after = map;
	VariableMap before;
	do {
		before = after;
		after = applyVariableMap (before, before);
	} while (after != before);
	return after;
}

void Logic::printDebug (){
#ifndef NDEBUG
	std::cout << LOGID << "*** Logic Debug ***" << std::endl;
	std::cout << LOGID << "Debug level:       " << m_debugLevel << std::endl;
	std::cout << LOGID << "Do Occure Check:   " << m_doOccureCheck << std::endl;
	std::cout << LOGID << "Max Query Depth:   " << m_maxQueryDepth << std::endl;
	std::cout << LOGID << "Max Uniform Depth: " << m_maxUniformDepth << std::endl;
	
	unsigned int j = 0;
	for (std::map<std::string, unsigned int>::iterator i = m_calls.begin(); i!= m_calls.end(); i++){
		std::cout << j << " " <<  i->first << " was called " << i->second << " times " << std::endl;
		j++;
	}
#else
	std::cout << LOGID << "Debug info has been optimized away" << std::endl;
#endif
}

std::string Logic::encodeAllVariables (const std::string & x) {
	FastStringCollector output;
	int len = x.length();
	STACKSTRING (head_stack, hs, len);
	STACKSTRING (tail_stack, ts, len);
	ts.setTo (x);
	char underLine = '_';
	SString ul (&underLine, 1, 1);
	do {
		KIF::syntacticalSplitHeadTail(ts, hs, ts);
		if (KIF::isVariable(hs)){
			// output << encodeToInternVariable (hs); // crashes, std::string is temporary
			output << hs << ul;
		} else {
			output << hs;
		}
	} while (ts.length() > 0);
	return output.str();

	// slower version
//	KIF::SyntacticalTokenizer tokenizer (x);
//	std::ostringstream output;
//	std::string token;
//	while (tokenizer.next (token)){
//		if (KIF::isVariable(token)){
//			output << encodeToInternVariable (token);
//		} else output << token;
//	}
//	return output.str();
}

VariableMap Logic::encodeAllVariables (const VariableMap & map){
	VariableMap result; 
	for (VariableMap::const_iterator i = map.begin(); i!=map.end(); i++){
		result[encodeToInternVariable(i->first)] = i->second;
	}
	return result;
}

VariableMap Logic::decodeAllVariables (const VariableMap & map){
	VariableMap result;
	for (VariableMap::const_iterator i = map.begin(); i!=map.end(); i++){
		if (isInternVariable(i->first))		
			result[decodeFromInternVariable(i->first)] = i->second;
	}
	return result;
}

VariableMap Logic::combine (const VariableMap & a, const VariableMap & b){
	VariableMap result;
	for (VariableMap::const_iterator i = a.begin(); i!=a.end(); i++){
		result[i->first] = i->second;
	}
	for (VariableMap::const_iterator i = b.begin(); i!=b.end(); i++){
		result[i->first] = i->second;
	}
	return result;
}
