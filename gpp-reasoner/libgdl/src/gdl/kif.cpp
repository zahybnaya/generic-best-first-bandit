/*
 * kif.cpp
 *
 *  Created on: 12.11.2008
 *      Author: nos
 */

#include "kif.h"

#include <ctype.h>
#include "../logid.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <assert.h>

bool KIF::Tokenizer::next (std::string & token){
	m_lastPos = m_pos;
	while (m_pos < m_length){
		char c = m_s[m_pos];
		if (c == '(') m_braceDepth++;
		if (c == ')') m_braceDepth--;
		if ((c == ' ') && (m_braceDepth == 0)){
			m_pos++;
			break;
		}
		m_pos++;
	}
	if (m_pos==m_length){
		if (m_lastPos == m_pos) return false;
		token = m_s.substr (m_lastPos, m_pos - m_lastPos); // the very last token
		m_lastPos = m_pos;
		return true;
	}
	token = m_s.substr (m_lastPos, m_pos-m_lastPos-1); // -1  because of we do not want to return ' ' 
	return true;
}

bool KIF::Tokenizer::hasNext () const {
	size_t p = m_pos;
	unsigned int braceDepth = m_braceDepth;
	while (p < m_length){
		char c = m_s[p];
		if (c == '(') braceDepth++;
		if (c == ')') braceDepth--;
		if ((c == ' ') && (braceDepth == 0)){
			break;
		}
		p++;
	}
	if (p == m_length && m_lastPos == p) return false;
	return true;
}

bool KIF::SyntacticalTokenizer::next (std::string & token){
	if (m_onABrake){
		token = m_s[m_pos];
		m_onABrake = false;
		m_lastPos = m_pos + 1;
		m_pos++;
		return true;
	}
	while (m_pos < m_length){
		char c = m_s[m_pos];
		if (c == ' ' || c == ')' || c == '('){
			m_onABrake = true;
			break;
		}
		m_pos++;
	}

	if (m_pos==m_length){
		if (m_lastPos == m_pos) return false;
		token = m_s.substr (m_lastPos, m_pos - m_lastPos); // the very last token
		m_lastPos = m_pos;
		return true;
	}
	token = m_s.substr (m_lastPos, m_pos - m_lastPos);
	m_lastPos = m_pos;
	return true;
}

KIF::KIF (){
	m_braceLevel = 0;
	m_inputBuffer = new std::ostringstream ();
}

KIF::~KIF (){
	delete m_inputBuffer;
}

#define CHECK_PARAM_NUM(p2) if (args.size() != p2) { std::cerr << LOGID << "Param count mismatch in " << input << " near line num " << lineNum << std::endl; return false; }

bool KIF::parse (const std::string & line){
	std::string toParse = prepare (line);
	if (toParse == "") return true; // empty line
	// going through that line
	bool result = true;
	for (size_t i = 0; i < toParse.length(); i++){
		char c = toParse[i];
		// ignoring white spaces at brace Level 0 (because HttpMessages to rules directly after each other..)
		if (!(m_braceLevel == 0 && c == ' '))
			*m_inputBuffer << c;
		if (toParse[i] == '(') m_braceLevel++;
		if (toParse[i] == ')') {
			m_braceLevel--;
			if (m_braceLevel == 0){
				std::string candidate = prepareLine (m_inputBuffer->str());
				if (!checkSyntax (candidate)){
					std::cerr << LOGID << "Skipping " << candidate << " as syntax failed " << std::endl;
					continue;
				}
				bool subResult = parseFullLine (candidate);
				result = result && subResult;
				delete m_inputBuffer;
				m_inputBuffer = new std::ostringstream ();
			}
		}
	}
	return result;
}

bool KIF::parseFullLine (const std::string & input, int lineNum){
	std::string cmd;
	std::vector<std::string> args;
	bool suc = separateCommand (input, cmd, args);
	if (!suc) return false;

	if (cmd == "<="){
		m_clauses.push_back (input);
		return true;
	}
	// now it has to be a fact
	m_facts.push_back (input);
	return true;
}
	
bool KIF::parseFile (const std::string & f, bool ignoreErrors){
	std::ifstream input (f.c_str());
	if (input.bad()) return false;
	while (!input.eof()){
		std::string line;
		std::getline (input, line);
		bool state = parse (line);
		if (state == false && ignoreErrors == false) return false;
	}
	if (input.bad()) return false;
	return true;
}
	
std::ostream & KIF::print (std::ostream & ss) const{
	ss << "Facts:   " << std::endl << m_facts << std::endl;
	ss << "Clauses: " << std::endl << m_clauses << std::endl;
	return ss;
}

std::string KIF::stripEmpties (const std::string & s){
	size_t b = 0;
	while (b < s.length()){
		char c = s[b];
		if (c != ' ' && c != '\t' && c != '\n') break;
		b++;
	}
	size_t e = s.length() - 1;
	while (e >= 0){
		char c = s[e];
		if (c != ' ' && c != '\t' && c != '\n') break;
		e--;
	}
	if (b == s.length() || e == 0) return "";
	return (s.substr(b, e - b + 1));
}

std::string KIF::stripMiddleEmpties (const std::string & s){
	std::ostringstream output;
	bool lastWasEmpty = false;
	for (size_t i = 0; i < s.length(); i++){
		char c = s[i];
		if (c == '\t') c = ' ';
		if (c == ' '){
			if (i+1 < s.length() && s[i+1] == ')') continue; // ignore ' ' before ')'
			if (!lastWasEmpty) output << c;
			lastWasEmpty = true;
		} else {
			lastWasEmpty = false;							// ignore ' ' after '('
			output << c;
		}
		if (c == '(') lastWasEmpty = true;
	}
	return output.str ();
}

std::string KIF::insertBraceEmpties (const std::string & s){
	std::ostringstream output;
	if (s.length() == 0) return s;
	output << s[0];
	char l = s[0];
	for (size_t i = 1; i < s.length(); i++){
		char c = s[i];
		
		if (c == '(' && l!=' ' &&  l!= '(') {
			output << ' '; // insert ' ' before '('
			l = ' ';
		} else 
		if (c != ' ' && c !=')' && l== ')') {
			output << ' '; // insert ' ' after ')'
			l = ' ';
		} else {
			l = c;
		}
		output << c;
	}
	return output.str ();
}

std::string KIF::stripComments (const std::string & s){
	size_t b = s.find(';');
	if (b == s.npos) return s;
	return s.substr (0, b);
}

std::string KIF::lowerCase (const std::string & s){
	std::string result;
	result.resize(s.length());
	for (size_t i=0; i < s.length(); i++) { result[i] = tolower (s[i]);}
	return result;
}

std::string KIF::prepareLine (const std::string & s){
	return insertBraceEmpties (lowerCase (stripMiddleEmpties(s)));
}

std::string KIF::prepare (const std::string & s){
	return stripEmpties (stripComments(s));
}

std::string KIF::prepareHttpMessage(const std::string & s){
	return prepareLine (prepare(s));// just a guess; i think there won't be any comments inside
}

bool KIF::isNotBraced (const std::string & s){
	if (s.length() == 0) return true;
	if (s[0] == '(' || s[s.length()-1] == ')') return false;
	return true;
}

std::string KIF::stripBraces (const std::string & s){
	assert (!isNotBraced (s));
	return s.substr (1, s.length() - 2);
}

bool KIF::checkSyntax (const std::string & s){
	if (s.empty()) return false;
	int braceCount = s[0] == '(' ? 1 : 0;
	for (size_t i = 1; i < s.length(); i++){
		
		char l = s[i-1];
		char c = s[i];
		if (c == ';' || l == ';') {
			std::cerr << LOGID << "No comments allowed in " << s << std::endl;
			return false; // no comments allowed
		}
		if (c == ' ' && l == ' '){
			std::cerr << LOGID << "No double empty signs allowed in " << s << std::endl;
			return false;
		}
		if (c == '(') braceCount++;
		if (c == ')') braceCount--;
		if (l == ')' && c != ' ' && c != ')'){
			std::cerr << LOGID << "Did not found ' '/')' after ')'" << std::endl;
			return false;
		}
		if (c == '(' && l != ' ' && l != '('){
			std::cerr << LOGID << "Did not found ' '/'(' before '('" << std::endl;
			return false;
		}
		if (c == '\t'){
			std::cerr << LOGID << "No tab signs allowed " << std::endl;
			return false;
		}
	}
	if (braceCount != 0){
		std::cerr << LOGID << "Brace mismatch in " << s << std::endl;
		return false;
	}
	return true;
}

KIF::StringSet KIF::getVariables(const std::string & s){
	KIF::SyntacticalTokenizer tokenizer (s);
	StringSet result;
	std::string token;
	while (tokenizer.next(token)){
		if (KIF::isVariable(token)) result.insert(token);
	}
	return result;
}

bool KIF::hasVariables (const std::string & s) {
	KIF::SyntacticalTokenizer tokenizer (s);
	std::string token;
	while (tokenizer.next(token)){
		if (KIF::isVariable(token)) return true;
	}
	return false;
}

void KIF::splitHeadTail (const std::string & s, std::string & head, std::string & tail) {
	assert (isBraced (s));
	Tokenizer tokenizer (stripBraces (s));
	tokenizer.next(head);
	tail = std::string ("(") + tokenizer.getTail() + ")";
}


bool KIF::separateCommand (const std::string & input, std::string & cmd, std::vector <std::string> & args){
	
	if (input.size() < 3 || input[0] != '(' || input[input.length()-1] != ')') {
		std::cerr << LOGID << "Input " << input << " is not surrounded by braces or not big enough" << std::endl;
		return false;
	}
	const std::string withoutBraces = input.substr (1, input.length() - 2); 
	size_t sep = withoutBraces.find(' ');
	if (sep == withoutBraces.npos) { 
		// no arguments in string
		cmd = withoutBraces;
		return true;
	}
	cmd = withoutBraces.substr (0, sep);
	
	sep++;
	int depth = 0;
	// parsing parameters
	std::ostringstream o;
	while (sep < withoutBraces.length()){
		char c = withoutBraces[sep];
		if (c == '(') depth++;
		if (c == ')') depth--;
		if ((c == ' ') && (depth == 0)){
			args.push_back (o.str());
			o.clear ();
		}
		o << c;
		sep++;
	}
	if (depth != 0){
		std::cerr << LOGID << "Braces count mismatch in " << withoutBraces << std::endl;
	}
	if (o.str() != "") args.push_back (o.str());
	return true;
}

bool KIF::separateCommandName (const std::string & input, std::string & cmd){
	if (input.size () < 3 || input[0] != '(' || input[input.length() -1] != ')') return false;
	const std::string withoutBraces  =input.substr (1, input.length() - 2);
	size_t sep = withoutBraces.find(' ');
	if (sep == withoutBraces.npos) { 
		// no arguments in string
		cmd = withoutBraces;
		return true;
	}
	cmd = withoutBraces.substr (0, sep);
	return true;
}

bool KIF::check (const std::string & input, bool & isFact, bool & isClause, bool doCheckSyntax){
	if (doCheckSyntax && !checkSyntax(input)) return false;
	if (isNotBraced(input)){
		// then it must have only one token
		KIF::Tokenizer tokenizer (input);
		std::string token;
		bool result = tokenizer.next (token);
		if (!result) {
			std::cerr << LOGID << "No token in not-braced " << input << std::endl;
			return false;
		}
		result = tokenizer.next (token);
		if (result) {
			std::cerr << LOGID << "More than one token in not-braced " << input << std::endl;
			return false;
		}
		isFact   = true;
		isClause = false;
		return true;
	}
	KIF::Tokenizer tokenizer (stripBraces(input));
	std::string token;
	bool result = tokenizer.next (token);
	if (!result) {
		std::cerr << LOGID << "No token in braced " << input << std::endl;
		return false;
	}
	if (token == "<="){
		/// also check the next token
		result = tokenizer.next (token);
		if (!result){
			std::cerr << LOGID << "No next token in clause " << input << std::endl;
			return false;
		}
		// this is enough checking
		isFact   = false;
		isClause = true;
		return true;
	}
	isFact = true;
	isClause = false;
	return true;
}

std::ostream & operator<< (std::ostream & ss, const std::vector<std::string> & vec){
	if (vec.size() == 0) return ss << "[]";
	std::vector<std::string>::const_iterator i = vec.begin();
	ss << "[" << *i;
	i++;
	for (; i!= vec.end(); i++){
		ss << ", " << *i;
	}
	ss << "]";
	return ss;
}

std::ostream & operator<< (std::ostream & ss, const std::set<std::string> & set){
	if (set.size() == 0) return ss << "[]";
	std::set<std::string>::const_iterator i = set.begin(); 
	ss << "[" << *i;
	i++;
	for (; i != set.end(); i++){
		ss << ", " << *i;
	}
	ss << "]";
	return ss;
}


