/*
 * gdl.h
 *
 *  Created on: 11.11.2008
 *      Author: nos
 */

#ifndef GDL_H_
#define GDL_H_

#include "logic.h"
#include "kboptimizer.h"
#include "compressor.h"

/// Encapsulates all things necessary for using GDL and for creating a player
class GDL {
public:
	typedef Logic::StringVec StringVec;
	
	/// A state within the game
	class State {
	public:
		State ();
		State (const State & s);
		/// Generates a state out of a existing vector of facts
		State (const StringVec & facts);
		
		/// Generates a Hash Value
		unsigned int getHash () const { return m_hash; }

		State& operator=  (const State & state); ///< Asignment operator
		bool   operator== (const State & state) const; ///< Comparing operator
		
		/// Returns the content in form of facts
		const Logic::StringVec& getFacts () const { return m_facts;}
	
		std::ostream & operator<< (std::ostream & ss) const;
	private:
		/// Sort the values (used before hashing)
		void sort ();
		void calcHash ();

		unsigned int 	 m_hash;	///< Hash value of the state
		StringVec m_facts;	///< The facts inside (sorted)
	};
	
	typedef StringVec Move;  ///< A Move (containing all moves from all Players)
	
	/// A Node for searching the tree
	struct Node {
		Node () { isTerminal = false; points = -1; }
		State state; 		///< the state
		Move  move;  		///< Move (leading to this node)
		bool  isTerminal;	///< is this a final state?
		
		int points;			///< how many points did it get? (user set; we don't care here for which player we are)
		int depth;			///< Depth of the node
		std::ostream & operator<< (std::ostream & ss) const;
		/// Returns hash value from the state
		unsigned int getHash () { return state.getHash(); }
	};
	
	typedef std::vector<Node*> NodeVec;
	
	
	GDL ();
	~GDL ();
	
	/// init from a file, loads in data
	bool initFromFile (const std::string & filename);
	/// initializes from KIF-Data structure
	bool initFromKIF (const KIF & kif);
	/// Returns initial state
	State getInitState () const;
	/// Returns the roles names
	const StringVec& getRoles () const;
	/// Returns the number of roles
	unsigned int getRoleCount () const;
	
	/// Returns possible moves for a given role
	StringVec getPossibleMoves (size_t role, const State & state) const;
	/// Calculates the next state with the given transaction move and the given state before
	State getNextState (const State & preState, const Move & move) const;
	/// checks whether state is a finish state
	bool isFinishState (const State & state) const;
	/// saves points in points; only valid if state is finish state; returns true on success
	bool points (size_t role, const State & state, int & points);
	/// en/disables debug mode
	void setDebug (bool v = true) { m_logic.setDebug (v); }
	/// print some debug info
	void printDebug () { m_logic.printDebug (); }
	
	// Working with Nodes
	/// Returns the initial node or zero if an error occurs
	/// delete the result for yourself
	Node * getInitialNode () const;
	
	/// expands a node (returns false on errors); target will contain all subnodes of all move combinations
	/// delete the nodes inside the nodevec for yourself
	/// before it starts it saves the possible moves of all roles in the current node in 'possibleMoves'
	/// the order of the children is like the following:
	/// e.g.
	/// if you have 3 roles; first role has 4 possibility (ABCD), second role 3 possibilites (012), third has 2 possibilities (XY)
	/// A0X, A0Y, A1X, A1Y, A2X, A2Y, B0X, B0Y, B1X, B1Y, B2X, B2Y ... D0X, D0Y, D1X, D1Y, D2X, D2Y (size=24)
	bool expand (Node * n, NodeVec & target, std::vector<StringVec> & possibleMoves) const;
	
	/// compatibility function for those who don't want to know the possible Moves
	bool expand (Node * n, NodeVec & target) const {
		std::vector<StringVec> possibleMoves;
		return expand (n, target, possibleMoves);
	}
	
	/// compresses the string into the internal representation
	/// (use it for communication with inside; if you enable compression)
	std::string compress (const std::string & s) const {
		if (!m_compressing) return s;
		else {
			std::string out;
			bool ret = m_compressor.compress (s, out);
			assert (ret);
			return out;
		}
	}
	
	/// decompressing the string into it's external representation
	/// (use it for communication with outside; if you enable compression)
	std::string decompress (const std::string & s) const {
		if (!m_compressing) return s;
		else {
			std::string out;
			bool ret = m_compressor.decompress (s, out);
			assert (ret);
			return out;
		}
	}
	
	/// returns current compression status
	bool compressingEnabled () { return m_compressing; }
	
	/// sets compression status, must be done before initialization
	void setCompressing (bool v) { assert (!m_initialized); m_compressing = v; }
	
	/// returns wether optimizer is enabled
	bool optimizingEnabled () { return m_optimizing; }
	
	/// sets optimizing state, must be done before initialisation
	void setOptimizing (bool v) { assert (!m_initialized); m_optimizing = v; }

private:
	/// Calculates the next node from the fact base of a current one
	/// baseNode->the base node
	/// nextNode->move must be already set
	/// currentKB is the KnowledgeBase of the baseNode (could be also calculcated by baseNode, but so it's faster)
	bool calcNextNode (Node * baseNode, Node * nextNode, KnowledgeBase * currentKB) const;
	
	
	typedef std::vector <Node*> NextNodeCalcList;
	
	/// Does calcNextNode onto all elements of the worklist (can be multithreaded)
	bool calcNextNodeWorkList (Node * baseNode, const NextNodeCalcList & worklist, KnowledgeBase * currentKB) const;

	/// Data shared between all threads
	struct ThreadData {
		bool error;	 						// some error happenend
		Node * baseNode;					// the basenode from which we all calculate
		KnowledgeBase * currentKB;			// KnowledgeBase derived from baseNode 
		const NextNodeCalcList * workList; 	// the working list
	};
	/// executes calcNext node; callable using the threadpool
	void calcNextNode (Node * baseNode, Node * nextNode, KnowledgeBase * currentKB, bool * error){
		if (!calcNextNode (baseNode, nextNode, currentKB)){
			*error = true;
		}
	}

	bool m_initialized;  ///< Is initialized yet?
	StringVec m_roles; ///< The names of the roles
	Logic     m_logic;   ///< The backing logic
	KnowledgeBase m_kb; ///< The rules for the game
	
	/// Formats a role move into a KnowledgeBase entry
	std::string formatMove (size_t roleId, const std::string & move) const {
		std::ostringstream os;
		os << "(" + compress("does") + " " << m_roles[roleId] << " " + move + ")";
		return os.str();
	}
		
	Compressor m_compressor;
	bool m_compressing; ///< compressing enabled
	bool m_optimizing;	///< Use KBOptimizer
};

inline std::ostream & operator<< (std::ostream & ss, const GDL::State & s) { return s.operator<<(ss); }
inline std::ostream & operator<< (std::ostream & ss, const GDL::Node & n) { return n.operator<<(ss); } 

#endif /* GDL_H_ */
