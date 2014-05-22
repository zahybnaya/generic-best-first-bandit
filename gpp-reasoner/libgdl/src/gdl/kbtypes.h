/*
 * kbtypes.h
 *
 *  Created on: 21.01.2009
 *      Author: nos
 */

#ifndef KBTYPES_H_
#define KBTYPES_H_

#include "kif.h"
#include <map>
#include <list>
#include <deque>
#include <boost/shared_ptr.hpp>

/*
 * Some types used by KnowledgeBase and Optimzier
 * Defines important things like Fact, Clause and so on
 */

/// Defines a relation name
struct RelationSignature {
	RelationSignature () : argc (0) {}
	/// Initializes the Relation from it's signature. Special recognition of (true/1) written as true_InnerName/X
	RelationSignature (const std::string & relation);
	std::string name;	///< Name
	int argc;			///< Num of arguments
	/// Comparison operator (for std::map)
	bool operator< (const RelationSignature & other) const { 
		if (name < other.name) return true;
		if (other.name < name) return false;
		return argc < other.argc;
	}
};

/// comparison operator for signatures
inline bool operator== (const RelationSignature & a, const RelationSignature & b) {
	return (a.argc == b.argc && a.name == b.name);
}

inline bool operator!= (const RelationSignature & a, const RelationSignature & b){
	return !(a == b);
}

typedef std::set<RelationSignature> RelationDependency;
typedef std::vector<RelationDependency> RelationDependencies;
typedef std::set<std::string> StringSet;
typedef std::deque <std::string> StringDeque;



/// Info about a relation (only used in analyzing through KBOptimizer)
struct RelationInfo {
	RelationInfo () : factWithVariables (false), impact (0), factCount (0), clauseCount (0), specialType(0) {}
	static const int REGULAR = 0;
	static const int LINEAR = 1;
	static const int SMALLER = 2;
	// static const int SMALLER_OR_EQUAL = 3;
	static const int GREATER = 4;
	// static const int BIGGER_OR_EQUAL = 5;
	bool factWithVariables;			///< there is at least one fact containing variables
	long impact;					///< how hard the function may be to calculate (guessed, mutliplicated combinations)
	int factCount;					///< how many facts found
	int clauseCount;				///< how many clauses found
	int specialType;				///< whether it is a special type (see above, regular, linear, smaller, smaller_or_equal...)
	StringDeque linearChain;		///< Only valid when specialType = Linear, Smaller or Greater. Gives the linearized order of the symbols
	RelationDependencies dependencies; ///< Dependencies of this Relation
};

struct RelationMap : public  std::map<RelationSignature, RelationInfo> {
	/// Searchres the signature inside the map; when it doesn't exist; then create it
	RelationInfo & findOrAdd (const RelationSignature & sig){
		RelationMap::iterator i = find(sig);
		std::map<RelationSignature, RelationInfo> & me = *this;
		if (i == end()){
			me[sig] = RelationInfo ();
			return me[sig];
		} return i->second;
	}
	
	/// gets impact of a specific signature within the map; returns 1 if not found
	/// you have to calculate the impacts first
	int getImpact (const RelationSignature & sig) const {
		const_iterator i = find(sig);
		if (i == end()) return 1;
		return i->second.impact;
	}
};

/// A Fact
struct Fact {
	Fact () {}
	Fact (const std::string & fact);
	std::string  name;
	std::string  text; // the original text of the fact (workaround for the Logic system to unify against)
	std::vector<std::string>  args;
	RelationSignature signature ();
	/// synthesizes text (useful when you generated name and args)
	void genText ();
};

/// A premiss (inside a clause) - for dynamic reordering
struct Premiss {
	Premiss () : isNegated(false), isOred(false), isDistinct(false), impact(0) {}
	Premiss (const std::string & base);
	/// Returns the bound variables in this premiss
	/// negated/distinct do not return variables
	/// ored only the ones which are in both
	StringSet boundVariables () const;
	
	bool isNegated;		// is a negated one
	bool isOred;		// is a ored one
	bool isDistinct;	// is a distinct
	int impact;			// calculated impact
	std::string text;	// the text of the premiss - for easy reconstruction
};

/// A parsed clause
struct Clause {
	/// empty constructor
	Clause () : valid(false) {}
	/// constructs claus	e from given written base
	Clause (const std::string & base);
	/// sets the impact values, from a given RelationMap)
	void calcImpact (const RelationMap & relationMap);
	/// reorders the Clause
	void reorder ();
	/// Clause has special premisses which need further threadment (negated or distinct)
	bool hasSpecialPremisses () const;
	/// fixes neg/distinct order (only necessary when hasSpecialPremisses, but checks it also another time)
	void fixNegDistinctConstraint ();
	/// prints the clause (in its current form)
	std::string toString () const;
	
	std::string head;				///< the head of the clause
	typedef std::list<Premiss> PremissList;
	PremissList premisses;			///< the premisses of the clause
	bool valid;						///< gone through constructor

	/// simple order predicate for sorting against impact, note that you need special care for some premisses
	struct PremissOrderPred {
		bool operator() (const Premiss & premiss1, const Premiss & premiss2) const {
			return premiss1.impact < premiss2.impact;
		}
	};
	
	RelationSignature signature () const { return RelationSignature (head); }
};

/// A map from variables to values
typedef std::map<std::string, std::string> VariableMap;
/// output operator for VariableMap
std::ostream & operator<< (std::ostream & o, const VariableMap & map);

/// API for answers of the logic system
class Answer {
public:
	virtual ~Answer () {}
	virtual bool next () = 0;								///< go to the next result
	virtual const VariableMap & getQuestionMap () = 0;		///< Returns the variable mapping in which this solution is valid
	/// Generates a fancy backtrace of the current answer
	virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth = 0) const { return bt << "Not implemented"; }
};

/// An answer with one possibility given before asking
class SingleAnswer : public Answer {
public:
	SingleAnswer (const VariableMap & qmap, const std::string & question = ""){
		m_qMap = qmap;
		m_gaveResult = false;
		// std::cout << LOGID << "Answering " << question << " with " << m_qMap << std::endl;
	}
	
	virtual bool next () {
		bool toReturn = !m_gaveResult;
		m_gaveResult = true;
		return toReturn;
	}
	
	virtual const VariableMap & getQuestionMap () { return m_qMap; }

	
	virtual std::ostream & backtrace (std::ostream & s, unsigned int depth) { return s << "(" << depth << ") fixed one answer " << m_qMap << std::endl; }
private:
	bool m_gaveResult;
	VariableMap m_qMap;
};

/// An answer with no results found (but maybe with fuzzy value!)
class NoAnswer : public Answer {
public:
	NoAnswer (const std::string & question = "") {
		// std::cout << LOGID << "Answering " << question << " with NO result" << std::endl;
	}
	virtual bool next () { return false; }
	virtual const VariableMap & getQuestionMap () { return emptyQMap; }
	virtual std::ostream & backtrace (std::ostream & s, unsigned int depth) { return s << "(" << depth << ") fixed no answer"; }
private:
	static const VariableMap emptyQMap;
	
};

/// An enhancement for KnowledgeBase which can answer special questions by creating a Answer
/// This is used e.g. by KBOptimizer to optimize several relations
class Responder {
public:
	virtual ~Responder () {}
	/// Returns the signature for question this responder can answer
	virtual RelationSignature getSig () const = 0;
	/// Checks a question for applicability (it has also to fulfil a check against the signature)
	virtual bool applicable (const std::string & question) const = 0;
	/// Returns a answer to the question. This is called by Logic
	virtual Answer * ask (const std::string & question, unsigned int depth) const = 0;
};
typedef boost::shared_ptr<Responder> ResponderPtr;



inline std::ostream & operator<< (std::ostream & o, const RelationSignature & sig) { return o << sig.name << "/" << sig.argc; }
std::ostream & operator<< (std::ostream & o, const RelationInfo & info);
std::ostream & operator<< (std::ostream & o, const RelationMap & relMap);
std::ostream & operator<< (std::ostream & o, const Fact & fact);
std::ostream & operator<< (std::ostream & o, const Clause & clause);
std::ostream & operator<< (std::ostream & o, const Premiss & premiss);
std::ostream & operator<< (std::ostream & o, const StringDeque & deque);


#endif /* KBTYPES_H_ */
