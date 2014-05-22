/*
 * kbtypes.cpp
 *
 *  Created on: 21.01.2009
 *      Author: nos
 */

#include "kbtypes.h"
#include <assert.h>
#include "../tools.h"
#include "../faststringcollector.h"

RelationSignature::RelationSignature (const std::string & relation){
	if (KIF::isNotBraced(relation)){
		name = relation;
		argc = 0;
	} else {
		KIF::Tokenizer tokenizer (KIF::stripBraces(relation));
		// searching name
		bool result = tokenizer.next (name);
		assert (result);
		std::string token;
		// counting arguments
		argc = 0;
		while (tokenizer.next(token)) { argc++; }
	}
}

Fact::Fact (const std::string & fact){
	text = fact;
	if (KIF::isNotBraced(fact)){
		name = fact;
		// args stays clear
	} else {
		KIF::Tokenizer tokenizer (KIF::stripBraces(fact));
		// searching name
		bool result = tokenizer.next (name);
		assert (result);
		std::string token;
		// counting arguments
		while (tokenizer.next(token)) { 
			args.push_back (token); 
		}
	}
}

RelationSignature Fact::signature () {
	RelationSignature sig; 
	sig.name = name;
	sig.argc = args.size(); 
	return sig;
}

void Fact::genText () { 
	if (args.size() == 0){
		text = name;
	} else {
		FastStringCollector output;
		SString open  ("(", 1, 1);
		SString close (")", 1, 1);
		SString empty (" ", 1, 1);
		output << open;
		output << name;
		for (std::vector<std::string>::const_iterator i = args.begin(); i != args.end(); i++){
			output << empty;
			output << *i;
		}
		output << close;
		text = output.str ();
	}	
}

Premiss::Premiss (const std::string & base) : isNegated(false), isOred(false), isDistinct(false), impact(0) {
	text = base;
	if(KIF::isNotBraced(base)){
		return;
	}
	KIF::Tokenizer tokenizer (KIF::stripBraces(base));
	std::string token;
	bool result = tokenizer.next (token);
	if (!result){
		std::cerr << LOGID << "strange premiss " << base << std::endl;
		return;
	}
	if (token == "not"){
		isNegated = true;
	}
	if (token == "or"){
		isOred = true;
	}
	if (token == "isDistinct"){
		isDistinct = true;
	}
}

StringSet Premiss::boundVariables () const {
	if (isNegated || isDistinct){
		return StringSet ();
	}
	if (isOred){
		KIF::Tokenizer tokenizer (KIF::stripBraces(text));
		std::string token;
		tokenizer.next(token);
		assert (token == "or");
		tokenizer.next (token);
		KIF::StringSet common = KIF::getVariables(token);
		while (tokenizer.next(token)){
			common = KIF::setIntersection (common, KIF::getVariables(token));
		}
		return common;
	}
	return KIF::getVariables(text);
}


Clause::Clause (const std::string & base){
	valid = false;
	if (KIF::isNotBraced(base)){
		std::cerr << LOGID << "Error with " << base << std::endl;
		return;
	}
	KIF::Tokenizer tokenizer (KIF::stripBraces(base));
	std::string token;
	bool result = tokenizer.next(token);
	if (!result || token != "<="){
		std::cerr << LOGID << base << " is not a clause" << std::endl;
		return;
	}
	
	result = tokenizer.next(token);
	if (!result){
		std::cerr << LOGID << base << " has no head" << std::endl;
		return;
	}
	head = token;
	while (tokenizer.next(token)){
		Premiss premiss (token);
		premisses.push_back (premiss);
	}
	valid = true;
}

void Clause::calcImpact(const RelationMap & relationMap){
	for (PremissList::iterator i = premisses.begin(); i!=premisses.end(); i++){
		Premiss & p = *i;
		if (p.isDistinct){
			p.impact = 1;
		} else 
		if (p.isNegated){
			// negated can take much longer; so we assume it takes as much as not negated...
			KIF::Tokenizer tokenizer (KIF::stripBraces(p.text));
			std::string token;
			bool result = tokenizer.next(token);
			assert (result && token == "not");
			result = tokenizer.next(token);
			assert (result);
			p.impact = relationMap.getImpact (RelationSignature(token));
		} else
		if (p.isOred){
			// get the subnodes..
			KIF::Tokenizer tokenizer (KIF::stripBraces(p.text));
			std::string token;
			bool result = tokenizer.next(token);
			assert (result && token == "or");
			p.impact = 0;
			// std::cout << "subtarget " << p.text << " of " << head << " is ored; calcing impact" << std::endl;
			while (tokenizer.next(token)){			
				RelationSignature subSig (token);
				p.impact += relationMap.getImpact (subSig);
				// std::cout << "adding " << getImpact (subSig, relationMap) << " as impact of " << subSig << std::endl;
			}
			// std::cout << "so having " << p.impact << std::endl;
		} else {
			// regular
			p.impact = relationMap.getImpact (RelationSignature(p.text));
		}
	}
}

void Clause::reorder(){
	/*
	 * First sorting, then bubbling down negation and distinct after variable instantiation
	 */
	// std::cout << "Before: " << toString () << std::endl;
	PremissOrderPred pred;
	premisses.sort(pred);
	fixNegDistinctConstraint();
}

void Clause::fixNegDistinctConstraint() {
	if (!hasSpecialPremisses()) return;
	StringSet boundVariables;
	PremissList::iterator i = premisses.begin();
	while (i != premisses.end()){
		const Premiss & p = *i;
		boundVariables = KIF::setUnion (boundVariables, p.boundVariables());
		// std::cout << "Bound Variables after " << p << " are " << boundVariables << std::endl;
		if (p.isDistinct || p.isNegated){
			StringSet variables = KIF::getVariables (p.text);
			if (!KIF::isSubset (variables, boundVariables)){
				// std::cout << "Variables of " << p.text << " = " << variables << " are not bound, moving it" << std::endl;
				PremissList::iterator next (i); next++;
				if (next == premisses.end()){
					std::cerr << LOGID << "Cannot walk " << p << " one step further, has not all variables bound" << std::endl;
					i++;
					continue;
				}
				std::swap(*i, *next);
			} else i++;
		} else i++;
	}
}

bool Clause::hasSpecialPremisses() const {
	for (PremissList::const_iterator i = premisses.begin(); i!= premisses.end(); i++){
		const Premiss & p = *i;
		if (p.isNegated || p.isDistinct) return true;
	}
	return false;
}

std::string Clause::toString() const{
	std::ostringstream os;
	os << "(<= " << head;
	for (std::list<Premiss>::const_iterator i = premisses.begin(); i!= premisses.end(); i++){
		os << " " << i->text;
	}
	os << ")";
	return os.str();
}

std::ostream & operator<< (std::ostream & o, const RelationInfo & info){
	o << "\timpact:            " << info.impact << std::endl;
	o << "\tfactCount:         " << info.factCount << std::endl;
	o << "\tclauseCount:       " << info.clauseCount << std::endl;
	o << "\tfactWithVariables: " << info.factWithVariables << std::endl;
	o << "\tdependencies:      ";
	for (RelationDependencies::const_iterator i = info.dependencies.begin(); i!= info.dependencies.end(); i++){
		o << "[";
		for (RelationDependency::const_iterator j = i->begin(); j!= i->end(); j++){
			o << *j << " ";
		}
		o << "]";
	}
	o << std::endl;
	return o;
}

std::ostream & operator<< (std::ostream & o, const RelationMap & relMap) {
	o << "RelationMap {" << std::endl;
	for (RelationMap::const_iterator i = relMap.begin(); i!=relMap.end(); i++){
		o << i->first << std::endl;
		o << i->second << std::endl;
	}
	o << "}" << std::endl;
	return o;
}

std::ostream & operator<< (std::ostream & o, const Fact & fact){
	if (fact.args.size() == 0){
		o << "Fact { " << fact.name << std::endl;
	} else {
		o << "Fact { (" << fact.name;
		for (std::vector<std::string>::const_iterator i = fact.args.begin(); i!= fact.args.end(); i++){
			o << " " << *i;
		}
		o << ")";
	}
	return o;
}

std::ostream & operator<< (std::ostream & o, const Clause & clause){
	o << "Clause {" << clause.head << " <= ";
	for (Clause::PremissList::const_iterator i = clause.premisses.begin(); i!= clause.premisses.end(); i++){
		const Premiss & p = *i;
		o << p << " ";
	}
	o << "}";
	return o;
}

std::ostream & operator<< (std::ostream & o, const Premiss & p){
	o << p.text << " impact: " << p.impact << " (neg/or/distinct) " << p.isNegated << "/" << p.isOred << "/" << p.isDistinct;
	return o;
}

std::ostream & operator<< (std::ostream & o, const StringDeque & deque){
	if (deque.empty()) return o << "[]";
	StringDeque::const_iterator i = deque.begin();
	o << "[" << *i;
	i++;
	for(; i != deque.end(); i++){
		o << ", " << *i;
	}
	o << "]";
	return o;	
}

std::ostream & operator<< (std::ostream & o, const VariableMap & map) {
	if (map.size() == 0) return (o << "[]");
	VariableMap::const_iterator i = map.begin();
	o << i->first << " --> " << i->second;
	i++;
	for (; i!=map.end(); i++){
		o << ", " << i->first << " --> " << i->second;
	}
	return o;
}



