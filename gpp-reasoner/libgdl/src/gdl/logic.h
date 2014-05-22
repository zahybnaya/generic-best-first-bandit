/*
 * logic.h
 *
 *  Created on: 12.11.2008
 *      Author: nos
 */

#ifndef LOGIC_H_
#define LOGIC_H_
#include "kif.h"
#include <map>
#include <string>
#include <deque>
#include "knowledgebase.h"


/** A simple logic processor.
    It uses the lisp like syntax of the KIF module with simplifications
    also used in GDL, so no cut, only a few hard-coded relations (or, distinct, not)
    
    It won't be very optimized
    
    TODO: 
    * Encapsulating the different optimized Answering versions, maybe using a factory
    * ClauseAnswer needs rewrite; is ugly to read, but seems to work
    * Putting all Answer types in Responder (KnowledgeBase::Responder), better design?
*/
class Logic {
public:
	
	typedef std::vector<std::string> StringVec;
	
	
	/// Renames variables in results from intern back into their real ones. Used
	/// As a decorator for answers which are directed to questions with intern variables only
	class AnswerDecoder : public Answer {
	public:
		/// Initializes Ques 
		AnswerDecoder (Answer * answer, const std::string & originalQuestion, const KnowledgeBase & kb) : m_kb (kb), m_originalQuestion (originalQuestion), m_wasTrueTimes (0) {
			m_answer = answer;
		}
		
		/// Deletes question encoder and also the hidden answer
		~AnswerDecoder (){
			delete m_answer;
		}
		
		/// Returns the encoded question (you need it to create the real answer)
		static std::string getEncodedQuestion (const std::string & question, unsigned int depth){ return encodeAllVariables (question); }

		virtual bool next ();
		virtual const VariableMap & getQuestionMap () { return m_questionMap; }
		virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth) const { return m_answer->backtrace(bt, depth); }
	private:
		VariableMap m_questionMap;
		Answer * m_answer;
		const KnowledgeBase & m_kb;	///< KnowledgeBase (for hinting purposes)
		std::string m_originalQuestion; ///< Original question (for hinting purposes)s
		int m_wasTrueTimes;			///< How often it was true (for caching purposes)
	};

	
	/// A version of QueryAnswer which shall used when asking against facts AND clauses
	class ClauseAnswer : public Answer {
	public:
		virtual ~ClauseAnswer ();
		
		virtual bool next ();										///< go to the next result
		// const VariableMap & getHeadMap () { return m_headMap;}		///< Returns the current head map for this solution
		virtual const VariableMap & getQuestionMap () { return m_questionMap;}		///< Returns questionMap
		virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth) const;
	protected:
		friend class Logic;
		ClauseAnswer (const Logic & l, const RelationSignature & sig, const std::string & question, const KnowledgeBase & kb, unsigned int depth = 0);
		// ClauseAnswer (const Logic & l, const std::string & question, const KnowledgeBase & kb, unsigned int depth = 0);

		struct SubAnswer {
			Clause::PremissList::const_iterator nextPremiss;	///< points to the next premiss to be answered
			Answer * partAnswer;								///< Answer* instance at this point
			VariableMap headMap;								///< Head map until this
		};
		size_t 			m_position;						///< Position of the fact for next answer to check
		
		const Logic & m_logic;								///< Access to logic system
		const KnowledgeBase & m_kb;						///< The used knowledgebase
		const std::string m_question;					///< Initial question
		const RelationSignature m_sig;					///< The signature of the question (given on construction)
		// std::string m_questionHead;					///< First token of the question
		std::deque<SubAnswer> m_subAnswers;				///< The sub answers within one clause
		
		KIF::Tokenizer 	m_clauseTokenizer;				///< A tokenizer for the clause
		VariableMap		m_questionMap;					///< The map of the question (for returning, not for actual use)
		size_t          m_clauseLength;					///< Length of current clause
		unsigned int 	m_depth;						///< Depth during lookup
		bool            m_onAnAnswer;					///< is currently on an answer
		bool			m_isFactAnswer;					///< Answer is based on facts
	};
	
	/// Answers Questions with or
	class OrClauseAnswer : public Answer {
	public:
		OrClauseAnswer (const Logic & l, const std::string & question, const KnowledgeBase & kb, unsigned int depth = 0);
		~OrClauseAnswer ();
		virtual bool next ();
		virtual const VariableMap & getQuestionMap () { return m_questionMap; }
		virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth) const;
	protected:
		const Logic	      &	m_logic;
		const KnowledgeBase & m_kb;						///< Knowledge Base
		KIF::Tokenizer m_tokenizer;		 				///< Current answer
		Answer *       m_currentAnswer;		  			///< Current sub Answer 
		VariableMap m_questionMap;			  			///< Question map for returning
		unsigned int m_depth;							///< Lookup depth
	};
	
	/// Simple distinct comparison (variables must have been bound all, i won't search new variables for you)
	class DistinctAnswer : public Answer {
	public:
		DistinctAnswer (const Logic & l, const std::string & q, const KnowledgeBase & kb, unsigned int depth = 0);
		virtual bool next ();
		virtual const VariableMap & getQuestionMap () { return m_questionMap; }
		virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth) const;
	protected:
		const Logic & m_logic;
		bool m_returnedResult;
		VariableMap m_questionMap;
		bool m_distinct; // is it distinct?
		KIF::Tokenizer m_tokenizer;
	};
	
	/// Simple not answer algorithm, all necessary variables have to be bound...
	class NotAnswer : public Answer {
	public:
		NotAnswer (const Logic & l, const std::string & q, const KnowledgeBase & kb, unsigned int depth=0);
		virtual ~NotAnswer ();
		virtual bool next ();
		virtual VariableMap & getQuestionMap () { return m_questionMap; }
		virtual std::ostream & backtrace (std::ostream & bt, unsigned int depth) const;
	protected:
		const Logic	      &	m_logic;
		Answer      	   * m_subAnswer;
		const std::string  & m_question;
		bool m_returnedResult;
		VariableMap m_questionMap;
		bool m_not;
	};
	
	Logic ();
	~Logic ();
	
	/// Ask the system a question (this is the only source for optimized ones) 
	/// delete the result for yourself!
	Answer * ask (const std::string & question, const KnowledgeBase & kb, unsigned int depth = 0) const;

	/// Asks a question and replaces the variables from the question with the results
	/// Does not check for double results if you set checkForDoubles to true
	/// If checkForDoubles is set to true, the result will also be sorted...
	StringVec simpleAsk (const std::string & question, const std::string & pattern, const KnowledgeBase & kb, bool checkForDoubles = false) const;

	/// Checks wheather something /can/ be satisified (not interested in variables)
	bool isSatisfiable (const std::string & question, const KnowledgeBase & kb) const;
	
	/// calls unify_mgu and compresses the result on success...
	bool unify_mgu_compressed (const std::string & x, const std::string & y, VariableMap & mgu, unsigned int depth = 0) const;
	
	/// does unification process with common variable map (need to rename variable names!)
	/// mgu in reference
	bool unify_mgu (const SString & x, const SString & y, VariableMap & mgu, unsigned int depth = 0) const;
	
	/// unifies variable var to the given value (mguvar in reference)
	bool unify_mguvar (const SString & var, const SString & value, VariableMap & mgu, unsigned int depth = 0) const;
	
	/// occure check, checks whether var is in value
	bool unify_mgucheck (const SString & var, const SString & value, VariableMap & mgu, unsigned int depth = 0) const;
	
	/// enables/disables debug mode
	void setDebug (bool v = true) const { m_debugLevel = v?1 : 0; /* allowed as m_debugLevel is mutable */}
	/// debug mode set?
	bool isDebug () const { return m_debugLevel > 0;}
	/// set debug level (0 .. no debug, 1 .. a bit, 2 .. also unification)
	void setDebugLevel (int v = 1) const { m_debugLevel = v; /* allowed as m_debugLevel is mutable */}
	/// returns debug level;
	int getDebugLevel () const { return m_debugLevel;}
	
	/// enables/disables occure check
	void setOccureCheck (bool v = true) { m_doOccureCheck = v; }
	/// occure check set?
	bool isOccureCheck () const { return m_doOccureCheck; }
	
	
	/// Apply a variable map on to a string
	static std::string applyVariableMap (const VariableMap & map, const std::string & base);
	/// Apply a variable map (map) onto another (base)
	static VariableMap applyVariableMap (const VariableMap & map, const VariableMap & base);
	/// replaces a variable (var) inside from with to
	static std::string applyVariableMap (const std::string & var, const std::string & to, const std::string & base);
	/// Applies a variable map to itself as long it is self-referencing
	static VariableMap compressVariableMap (const VariableMap & map);
	
	/// print some debug info
	void printDebug ();
	
private:
	/// encodes a variable name to mark it as a intern one
	static std::string encodeToInternVariable (const std::string & var) { return var + "_"; }
	
	/// Returns true if variable var is a intern variable (assumes that it is a variable)
	static bool isInternVariable (const std::string & var) { return var[var.length()-1] == '_'; }
	
	/// decodes a intern variable back into it's official name
	static std::string decodeFromInternVariable (const std::string & var) { return var.substr (0, var.length() - 1); }
	
	/// encodes all variables into intern ones in a given string
	static std::string encodeAllVariables (const std::string & x);
	
	/// encodes all variables into intern ones in a given variable map (only at the beginners - no self referencing)
	static VariableMap encodeAllVariables (const VariableMap & map);
	
	/// decodes all variables into non-intern ones in a given variable map
	static VariableMap decodeAllVariables (const VariableMap & map);
	
	/// combines two (disjunct) variable maps
	static VariableMap combine (const VariableMap & a, const VariableMap & b);
	
	// StringVec m_facts;	// facts (copied from KIF)
	// StringVec m_clauses; // clauses (copied from KIF)
	unsigned int m_maxUniformDepth;     ///< maximum depth of uniform
	unsigned int m_maxQueryDepth;		///< maximum depth for queries
	mutable int m_debugLevel;			///< debug level, 0 -- no debug, 1 -- a bit, 2 -- also debugging unify
	bool m_doOccureCheck;				///< enabled occure check during unification
	
#ifndef NDEBUG
	mutable std::map<std::string, unsigned int> m_calls;
#endif	
};

#endif /* LOGIC_H_ */
