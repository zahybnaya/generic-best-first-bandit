/*
 * kif.h
 *
 *  Created on: 12.11.2008
 *      Author: nos
 */

#ifndef KIF_H_
#define KIF_H_

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>

#include "../stackstring.h"

/// A parser for the Knowledge Interchange Format (KIF)
/// It is used by Logic unit for getting initialized, later maybe by an optimized GDL version
class KIF {
public:

	/// An iterator for getting tokens in a string (ignores braced sub-tokens)
	/// note splitHeadTail is much faster!
	class Tokenizer {
	public:
		Tokenizer () : m_length(0), m_pos (0), m_lastPos (0), m_braceDepth (0) {} // empty string doen't have any tokens
		Tokenizer (const std::string & s) : m_s (s), m_length (s.length()), m_pos(0), m_lastPos(0), m_braceDepth(0) {}
		/// Scans for the next token; returns true on success and saves it in token
		bool next (std::string & token);
		/// Checks whether there is a next one without actually changing it; note it is faster to directly use next(std::string&s)
		bool hasNext () const;
		/// Returns the string which is tokenized
		const std::string & getString () const { return m_s; }
		/// returns the tail of the string (the rest)
		std::string getTail () const { return m_s.substr (m_pos, m_length - m_pos); }
	private:
		std::string m_s;	 ///< the string
		size_t m_length;	 ///< length of string
		size_t m_pos;		 ///< current (end) pos of token
		size_t m_lastPos;	 ///< current (begin) pos of token; was m_pos in last round
		int    m_braceDepth; ///< current brace depth; ignoring all whitespaces when >0
	};
	
	/// Another tokenizer which in fact returns the whole string (not depending on braces) but breaks when one token closes (at ' ' and ')' and '(')
	/// Note syntacticalSplitHeadTail is much faster
	class SyntacticalTokenizer {
	public:
		SyntacticalTokenizer () : m_length(0), m_pos (0), m_lastPos (0), m_onABrake(false) {};
		SyntacticalTokenizer (const std::string & s): m_s(s), m_length (s.length()), m_pos(0), m_lastPos(0), m_onABrake (false) {} 
		bool next (std::string & token);
	private:
		std::string m_s;	///< The string
		size_t m_length;	///< length 	
		size_t m_pos;		///< current pos
		size_t m_lastPos;	///< last current pos
		bool   m_onABrake;  ///< I'm currently on a brake sign
	};
	
	KIF ();
	~KIF ();
	
	/// inserts a file line into the look-ahead-buffer, removes comments. when all braces are closed it will get
	/// parsed by parseFullLine
	bool parse (const std::string & );
	
	/// parses a full line of a GDL (so all braces have to be closed yet)
	/// returns success state
	bool parseFullLine (const std::string &, int lineNum = -1);
	
	/// goes through a file
	bool parseFile (const std::string & f, bool ignoreErrors = false);
	
	/// gives a nice overview what is parsed yet
	std::ostream& print (std::ostream & o) const;
	
	/// Gains access to the facts of the form (name [param]*)
	const std::vector<std::string> & getFacts () const { return m_facts; }
	/// Gains access to the Clauses of the form (<= (Head) (Body1) [(BodyN)]* )
	const std::vector<std::string> & getClauses () const { return m_clauses; }
	
	// Useful toolbox functions
	/// strip empty chars (from begin and end)
	static std::string stripEmpties (const std::string & s);
	/// strip bad empty characters (e.g. multiple, or empties after '(' braces)
	static std::string stripMiddleEmpties (const std::string & s);
	/// insert empties after ) when follewed by ( and before ( if there is none
	static std::string insertBraceEmpties (const std::string & s);
	/// strip all comments
	static std::string stripComments (const std::string & s);
	/// converts string to lowercase (not, KIF is not case sensitive)
	static std::string lowerCase (const std::string & s);
	/// prepares a string (only one fact/clause allowed!) for parsing
	static std::string prepareLine (const std::string & s);
	/// prepares a subline of commands (here commands can be split over severall lines)
	static std::string prepare (const std::string & s);
	/// prepares a message send by http-client (used in GameHTTPServer)
	static std::string prepareHttpMessage (const std::string & s);
	
	/// Checks whether a string is not surrounded by braces like (blabla)
	static bool isNotBraced (const std::string & s);
	static bool isBraced (const std::string & s) { return !isNotBraced (s); }
	/// Strips braces from an string, DOES NO CHECKING WHEN NOT IN DEBUG MODE!!!!!
	static std::string stripBraces (const std::string & s);
	/** Checks syntax for further use
		- no empty string
		- no comments
		- correct brace count
		- maximum of one ' ' after each other
		- after ')' a ' ' or a ')'. After '(' no ' '
		- no tab signs
	*/ 
	static bool checkSyntax (const std::string & s);
	
	/// Returns true if a literal is a variable (begins with ?)
	static bool isVariable (const std::string & s) { return (s.length() > 0) && (s[0] == '?'); }
	
	typedef std::set<std::string> StringSet;
	/// Returns used variables inside the construct
	static StringSet getVariables (const std::string & s);
	/// Returns if there are any variable
	static bool hasVariables (const std::string & s);
	
	/// splits a braced string into head and tail (the first and the rest, yet again braced)
	/// if tail is empty it returns ()
	/// does asserting for wrong error
	static void splitHeadTail (const std::string & s, std::string & head, std::string & tail);

	/// separates a polish notation command into command name and it's arguments
	/// returns true on success
	static bool separateCommand (const std::string & input, std::string & cmd, std::vector<std::string> & args);

	/// extracts command name of a single braced input
	static bool separateCommandName (const std::string & input, std::string & cmd);
	
	/// check the type of a input. stores isFact/isClause if it's a fact or a clause
	/// returns true if it's valid
	/// if checkSyntax is set it will also call checkSyntax on it
	static bool check (const std::string& input, bool & isFact, bool & isClause, bool doCheckSyntax = false);
	
	// StringSet ops
	/// intersect operator
	static StringSet setIntersection (const StringSet & a, const StringSet & b){
		// http://www2.roguewave.com/support/docs/sourcepro/edition9/html/stdlibug/8-2.html
		StringSet intersection;
		std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), 
				std::inserter(intersection, intersection.begin()));
		return intersection;
	}

	/// union operator
	static StringSet setUnion (const StringSet & a, const StringSet & b){
		// http://www2.roguewave.com/support/docs/sourcepro/edition9/html/stdlibug/8-2.html
		StringSet u;
		std::set_union(a.begin(), a.end(), b.begin(), b.end(), 
				std::inserter(u, u.begin()));
		return u;
	}

	/// checks whether a is a subset of b
	static bool isSubset (const StringSet & a, const StringSet & b){
		return std::includes(b.begin(), b.end(), a.begin(), a.end());
	}

	// special operations on SString
	
	/// returns true if SString is not of the form (..)
	static bool isBraced (const SString & s){
		if (s.length() == 0) return false;
		return s[0] == '(' && s[s.length() - 1] == ')';
	}
	/// returns the opposite of isBraced
	static bool isNotBraced (const SString & s){ return !isBraced (s); }

	/// splits head and tail of SString
	/// note that head & tail must have at least the same size like s
	static void splitHeadTail (const SString & s, SString & head, SString & tail) {
		assert (isBraced(s));
		// searching the next ' ' at brace level 0
		int braceLevel = 0;
		int i;
		for (i = 1; i < s.length(); i++){
			const char & c = s[i];
			if (c == '(') braceLevel++;
			if (c == ')') braceLevel--;
			if ((c == ' ') && (braceLevel == 0)) break; // this is what we search
		}
		if (i == s.length()) {
			// there is only one left
			head.setTo (s.getData() + 1, s.length() - 2);
			tail.setTo ("()");
			return;
		}
		head.setTo (s.getData() + 1, i - 1);
		// putting the rest into tail
		tail.setTo (s.getData() + i, s.length() - i);
		tail[0] = '(';
	}
	
	/// same like splitHeadTail but syntactical (going from different token to different token)
	static void syntacticalSplitHeadTail (const SString & s, SString & head, SString & tail) {
		assert (s.length () > 0);
		if (s.length() == 0){
			head.setTo(0,0);
			tail.setTo(0,0);
		}
		int i;
		for (i = 0; i < s.length(); i++){
			char c = s[i];
			if (c == ' ' || c == '(' || c == ')') break;
		}
		if (i == 0) i = 1;
		// faster
		head = s.substr(0, i);
		tail = s.substr(i, s.length() - i);
		// head.setTo (s.getData(), i);
		// tail.setTo (s.getData() + i, s.length() - i);
	}
	
	/// checks whether s is a variable
	static bool isVariable (const SString & s) {
		return (s.length() > 0 && s[0] == '?');
	}
private:
	std::ostringstream * m_inputBuffer;
	int m_braceLevel;
	
	std::vector<std::string> m_facts;		///< Facts from the KIF
	std::vector<std::string> m_clauses;		///< Clauses from the KIF
};

std::ostream & operator<< (std::ostream & ss, const std::vector<std::string> & vec);
std::ostream & operator<< (std::ostream & ss, const std::set<std::string> & set);
inline std::ostream & operator<< (std::ostream & o, const KIF & kif) { return kif.print (o); } 


#endif /* KIF_H_ */
