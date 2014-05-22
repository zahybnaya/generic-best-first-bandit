/*
 * knowledgebase.h
 *
 *  Created on: 28.11.2008
 *      Author: nos
 */

#ifndef KNOWLEDGEBASE_H_
#define KNOWLEDGEBASE_H_
#include "kif.h"
#include <map>
#include <set>
#include "kbtypes.h"

/// KnowledgeBase saves facts and clauses and optimizes them in vectors for faster lookup
/// The design is made that you can put the KnowledgeBase onto a stack or something
class KnowledgeBase {
public:
	KnowledgeBase ();
	KnowledgeBase (const KIF & kif);
	
	typedef std::vector<Fact> FactVec;
	typedef std::vector<Clause> ClauseVec;
	typedef std::map<RelationSignature, FactVec>   FactMap;		///< Stored Facts accessing structure
	typedef std::map<RelationSignature, ClauseVec> ClauseMap;	///< Stored Clauses accesing structure
	
	
	typedef std::vector<std::string> StringVec;
	
	typedef std::map<RelationSignature, ResponderPtr> ResponderMap; ///< Stores responder for specific signatures
	
	/// Return facts beginning with a special signature
	const FactVec & getFacts (const RelationSignature & sig) const {
		FactMap::const_iterator i = m_facts.find(sig);
		if (i == m_facts.end()) return emptyFactVec;
		return i->second;
	}
	
	/// Return Clauses beginning with a special signature
	const ClauseVec & getClauses (const RelationSignature & sig) const {
		ClauseMap::const_iterator i = m_clauses.find (sig);
		if (i == m_clauses.end()) return emptyClauseVec;
		return i->second;
	}
	
	/// Access to all Facts (R/W)
	FactMap & getAllFacts () { flushCache (); return m_facts; }
	// Acces to all Facts 
	const FactMap & getAllFacts () const { return m_facts;}
	/// Access to all Responders (R/W)
	ResponderMap & getResponders () { return m_responders;}
	/// Access to all Responders
	const ResponderMap & getResponders () const { return m_responders; }
	
	/// Access to all Clauses (R/W)
	ClauseMap & getAllClauses () { flushCache (); return m_clauses; }
	/// Access to all Clauses
	const ClauseMap & getAllClauses () const { return m_clauses; }
	
	/// Adds new knowledge into the base
	void add (const std::string & s);
	/// Adds new knowledge into the base
	void add (const StringVec & vec);

	/// Returns a knowledge base with added knowledge
	inline KnowledgeBase added (const std::string & s) const { KnowledgeBase copy (*this); copy.add (s); return copy; }
	/// Returns a knowledge base with added knowledge
	inline KnowledgeBase added (const StringVec & vec) const { KnowledgeBase copy (*this); copy.add (vec); return copy; }
	
	std::ostream & operator<< (std::ostream & o) const;
	static std::ostream & print (std::ostream & o, const FactMap & km);
	static std::ostream & print (std::ostream & o, const ClauseMap & km);
	
	/// Size of the Knowledgebase
	size_t size () const { return m_themAll.size(); }
	
	/// Adds a custom responder. There is only one responder per signature allowed. Returns true if success
	bool addResponder (const ResponderPtr & responder);
	/// checks whether we can respond to a question with the given signature directly; if so it returns the answer
	/// otherwise it returns 0
	Answer * canRespond (const RelationSignature & sig, const std::string & question, unsigned int depth = 0) const;

	/// Gives the KnowledgeBase a hint that a given question failed.
	void failedHint (const std::string & question) const;
	/// Gives the KnowledgeBase a hint that only one result returned
	void onlyOneHint (const std::string & question, const VariableMap & vmap) const;
	
	static int numQuestions;			///< Number of questions
	static int failedCacheHits;				///< Number of cache Hits
	static int onlyOneCacheHits;		///< Number of only one cache hits
	static int responderHits;			///< Number of responder Hits
	
	const std::set<std::string> & getThemAll () const { return m_themAll;} ///< return all facts / clauses
private:
	/// Resets the cache.
	void flushCache () { m_failedCache.clear(); m_onlyOneCache.clear(); }
	
	FactMap m_facts;					///< our facts
	ClauseMap m_clauses; 				///< our clauses
	std::set<std::string> m_themAll; 	///< we do not want double facts; so we save them all to check them against
	StringVec m_emptyVec;   			///< gets returned if no results found
	ResponderMap m_responders;			///< stored responders
	
	static const FactVec emptyFactVec;		///< Get's returned if no results found
	static const ClauseVec emptyClauseVec;	///< Get's returned if no results found
	
	mutable std::set<std::string> m_failedCache;				///< a cache for questions which failed. must be cleared on each change
	mutable std::map<std::string, VariableMap> m_onlyOneCache;	///< a cache for questions which responded with only one answer
};

inline std::ostream & operator<< (std::ostream & o, const KnowledgeBase & kb) { return kb.operator<<(o); }
std::ostream & operator<< (std::ostream & o, const KnowledgeBase::FactVec & fv);
std::ostream & operator<< (std::ostream & o, const KnowledgeBase::ClauseVec & cv);

#endif /* KNOWLEDGEBASE_H_ */
