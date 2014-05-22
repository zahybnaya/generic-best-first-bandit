/*
 * kboptimizer.h
 *
 *  Created on: 14.01.2009
 *      Author: nos
 */

#ifndef KBOPTIMIZER_H_
#define KBOPTIMIZER_H_
#include "knowledgebase.h"
#include <list>

#include "kbtypes.h"
#include "logic.h"
#include "../tools.h"

/// A responder for linear relations
class LinearResponder : public Responder {
public:
	LinearResponder (const RelationSignature & signature, const StringDeque & chain);
	virtual ~LinearResponder () {}
	virtual RelationSignature getSig () const { return m_sig; }
	virtual bool applicable (const std::string & question) const;
	virtual Answer * ask (const std::string & question, unsigned int depth) const ;
private:
	StringDeque m_chain;
	RelationSignature m_sig;
};

/// A responder for comparison relations
class ComparisonResponder : public Responder {
public:
	/// Creates the comparison responder applicable for a given signature
	/// If isSmaller is true then it is a isSmaller comparison, otherwise a isBigger
	ComparisonResponder (const RelationSignature & signature, const StringDeque & chain, bool isSmaller);
	virtual ~ComparisonResponder () {}
	virtual RelationSignature getSig () const { return m_sig; }
	virtual bool applicable (const std::string & question) const;
	virtual Answer * ask (const std::string & question, unsigned int depth) const;
private:
	StringDeque m_chain;
	RelationSignature m_sig;
};


/**
 *  KBOptimizer aims to find out something useful about KnowledgeBases and tries to optimize queries onto them
 *  by rechaining the layout of clauses
 */
class KBOptimizer {
public:
	KBOptimizer(KnowledgeBase & kb) : m_kb (kb) {}
	~KBOptimizer();

	typedef KnowledgeBase::FactVec FactVec;
	typedef KnowledgeBase::FactMap FactMap;
	typedef KnowledgeBase::ClauseVec ClauseVec;
	typedef KnowledgeBase::ClauseMap ClauseMap;
	
	typedef KnowledgeBase::StringVec StringVec;
	
	// ********* Investigation functions ************
	
	/// collects all Relations and export them
	void exportRelations (RelationMap & relationMap) const;
	/// calculatete the impact values in a relationMap
	static void calcImpact (RelationMap & relationMap);

	// ********* Optimizing Functions ****************
	
	/// builds up a RelationMap and reorders the order in clauses
	/// Note; this did not really speed up the process; as most games were already much more optimized
	void optimizeOrder ();
	/// fix neg/distinct after regular premisses constraint
	/// (moves neg/distinct at the end of clauses; so that there are no unbound variables before them anymore)
	void fixNegDistinctConstraint ();

	/// optimize special relations (e.g. linear/transitive)
	void optimizeSpecialRelations ();

private:
	/// find linear relations
	void findLinearRelations (RelationMap & relationMap);
	/// find comparison relations
	void findComparisonRelations (RelationMap & relationMap);
	
	KnowledgeBase & m_kb;
	// for optimization purposes
	static const int MAX_IMPACT = 10000; 
};

#endif /* KBOPTIMIZER_H_ */
