/*
 * compressor.cpp
 *
 *  Created on: 29.03.2009
 *      Author: nos
 */

#include "compressor.h"
#include "../tools.h"
#include <sstream>

Compressor::Compressor() {
	// TODO Auto-generated constructor stub
	m_currToken    = "a";
	m_currVariable = "?a";
}

Compressor::~Compressor() {
	// TODO Auto-generated destructor stub
}

void Compressor::init(const KIF & kif, KnowledgeBase & out) {
	const char * reserved [] = {"not", "distinct", "or", "and", "<=", "(", ")", " ", 0 };
	for (const char ** c  = reserved; *c != 0; c++) {
		m_compressMap[*c] = *c;
		m_decompressMap[*c] = *c;
	}

	typedef std::vector<std::string> StringVec;
	
	// facts
	const StringVec & facts = kif.getFacts();
	for (StringVec::const_iterator i = facts.begin(); i != facts.end(); i++){
		out.add (initialCompress (*i));
	}
	// clauses
	const StringVec & clauses = kif.getClauses();
	for (StringVec::const_iterator i = clauses.begin(); i != clauses.end(); i++){
		out.add(initialCompress (*i));
	}
}

bool Compressor::compress(const std::string & in, std::string & out) const {
	std::ostringstream os;
	int len = in.length();
	STACKSTRING (head_stack, hs, len);
	STACKSTRING (tail_stack, ts, len);
	ts.setTo (in);
	while (ts.length() != 0) {
		KIF::syntacticalSplitHeadTail(ts, hs, ts);
		CompMap::const_iterator i = m_compressMap.find (hs);
		if (i == m_compressMap.end()) {
			std::cerr << LOGID << "Fatal Did not found " << hs << " in compress map" << m_compressMap << std::endl;
			return false;
		}
		os << i->second;
	}
	out = os.str();
	return true;
}

bool Compressor::decompress(const std::string & in, std::string & out) const {
	std::ostringstream os;
	int len = in.length();
	STACKSTRING (head_stack, hs, len);
	STACKSTRING (tail_stack, ts, len);
	ts.setTo (in);
	while (ts.length() != 0) {
		KIF::syntacticalSplitHeadTail(ts, hs, ts);
		CompMap::const_iterator i = m_decompressMap.find (hs);
		if (i == m_decompressMap.end()) {
			std::cerr << LOGID << "Fatal Did not found " << hs << " in decompress map " << m_decompressMap << std::endl;
			return false;
		}
		os << i->second;
	}
	out = os.str();
	return true;
}

/// initial compresses a string; filling compMap / decompMap
std::string Compressor::initialCompress (const std::string& e) {
		int len = e.length();
		STACKSTRING (head_stack, hs, len);
		STACKSTRING (tail_stack, ts, len);
		std::ostringstream os;
		ts.setTo (e);
		while (ts.length () != 0){
			KIF::syntacticalSplitHeadTail (ts, hs, ts);
			os << genCompressed (hs);
		}
		return os.str();
}


/// increments a letter, returns true if there is a carrying flag
bool incLetter (char & l){
	if (l == 'z'){
		l = '0';
		return false;
	} else if (l == '9'){
		l = 'a';
		return true;
	} else {
		l++;
		return false;
	}
}

void increment (std::string & x) {
	int start = KIF::isVariable(x) ? 1 : 0;
	for (int i = x.length() - 1; i >= start; i--){
		if (!incLetter(x[i])) return;
	}
	x = x + 'a';
}

std::string Compressor::genCompressed (const std::string & s) {
	CompMap::const_iterator i = m_compressMap.find (s);
	if ( i != m_compressMap.end()) return i->second;
	// generate a new one
	if (KIF::isVariable(s)) {
		while (reserved (m_currVariable)){
			increment (m_currVariable);
		}
		m_compressMap[s] = m_currVariable;
		m_decompressMap[m_currVariable] = s;
		return m_currVariable;
		
	} else {
		while (reserved(m_currToken)) {
			increment (m_currToken);
		}
		m_compressMap[s] = m_currToken;
		m_decompressMap[m_currToken] = s;
		return m_currToken;
	}
}
