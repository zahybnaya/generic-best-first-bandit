/*
 * knowledgebase.cpp
 *
 *  Created on: 28.11.2008
 *      Author: nos
 */

#include "knowledgebase.h"
#include "../tools.h"
#include "assert.h"

/// static definitions
const KnowledgeBase::FactVec KnowledgeBase::emptyFactVec;
const KnowledgeBase::ClauseVec KnowledgeBase::emptyClauseVec;

int KnowledgeBase::numQuestions  = 0;
int KnowledgeBase::failedCacheHits     = 0;
int KnowledgeBase::onlyOneCacheHits = 0;
int KnowledgeBase::responderHits = 0;

KnowledgeBase::KnowledgeBase (){
}

KnowledgeBase::KnowledgeBase (const KIF & kif){
	add (kif.getFacts());
	add (kif.getClauses());
}

void KnowledgeBase::add (const std::string & s){
	flushCache ();
	std::string prepare = KIF::prepare(KIF::prepareLine (s));
	if (m_themAll.find (prepare) != m_themAll.end()) return; // we have this knowledge already
	
	bool valid, isFact, isClause;
	valid = KIF::check(prepare, isFact, isClause, false);
	if (!valid){
		std::cerr << LOGID << "Invalid input detected: " << s << std::endl;
		return;
	}
	assert ((isFact && !isClause) || (!isFact && isClause));
	if (isFact){
		Fact f (prepare);
		RelationSignature sig = f.signature();
		m_facts[sig].push_back(f);
	}
	if (isClause){
		Clause c (prepare);
		RelationSignature sig = c.signature();
		m_clauses[sig].push_back (c);		
	}
	m_themAll.insert(prepare);
}

void KnowledgeBase::add (const StringVec & vec){
	for (StringVec::const_iterator i = vec.begin(); i!=vec.end(); i++)
		add (*i);
}

bool KnowledgeBase::addResponder (const ResponderPtr& res){
	RelationSignature sig = res->getSig();
	if (m_responders.find(sig) != m_responders.end()){
		std::cerr << LOGID << "There is already a responder registered for signature " << sig << std::endl;
		return false;
	}
	m_responders[sig] = res;
	return true;
}

Answer * KnowledgeBase::canRespond (const RelationSignature & sig, const std::string & question, unsigned int depth) const {
	numQuestions++;
	// do we have it inside the cache?
	{
		if (m_failedCache.find(question) != m_failedCache.end()) {
			failedCacheHits++;
			return new NoAnswer (question);
		}
	}
	{
		std::map<std::string, VariableMap>::const_iterator i = m_onlyOneCache.find (question);
		if (i != m_onlyOneCache.end()){
			onlyOneCacheHits++;;
			return new SingleAnswer (i->second, i->first);
		}
	}
	
	{
		// check our responding array...
		ResponderMap::const_iterator i = m_responders.find(sig);
		if (i != m_responders.end()){
			// std::cout << LOGID << "Found responder; let's see wether it can handle question " << question << std::endl;
			const ResponderPtr & resp = i->second;
			if (i->second->applicable (question)){
				// we have an responder which is applicable!!
				responderHits++;
				return resp->ask(question, depth);
			}
		}
	}
	// no responder found
	return 0;
}

void KnowledgeBase::failedHint (const std::string & question) const {
	m_failedCache.insert (question);
}

void KnowledgeBase::onlyOneHint (const std::string & question, const VariableMap & vmap) const {
	m_onlyOneCache[question] = vmap;
}

std::ostream & KnowledgeBase::operator<< (std::ostream & o) const{
	o << "KnowledgeBase { Facts: " << std::endl;
	print (o, m_facts);
	o << " Clauses: " << std::endl;
	print (o, m_clauses);
	o << "}";
	return o;
}

std::ostream & KnowledgeBase::print (std::ostream & o, const FactMap & fm) {
	for (FactMap::const_iterator i = fm.begin(); i!=fm.end(); i++){
		o << "*** Sig: " << i->first << " ***" << std::endl;
		o << i->second;
	}
	return o;
}

std::ostream & KnowledgeBase::print (std::ostream & o, const ClauseMap & cm) {
	for (ClauseMap::const_iterator i = cm.begin(); i!=cm.end(); i++){
		o << "*** Sig: " << i->first << " ***" << std::endl;
		o << i->second;
	}
	return o;
}

std::ostream & operator<< (std::ostream & o, const KnowledgeBase::FactVec & fv){
	for (KnowledgeBase::FactVec::const_iterator i = fv.begin(); i!= fv.end(); i++){
		o << "  " << *i << std::endl;
	}
	return o;
}

std::ostream & operator<< (std::ostream & o, const KnowledgeBase::ClauseVec & cv){
	for (KnowledgeBase::ClauseVec::const_iterator i = cv.begin(); i!= cv.end(); i++){
		o << "  " << *i << std::endl;
	}
	return o;
}
