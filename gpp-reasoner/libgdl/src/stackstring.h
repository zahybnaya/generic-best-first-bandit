/*
 * stackstring.h
 *
 *  Created on: 26.01.2009
 *      Author: nos
 */

#ifndef STACKSTRING_H_
#define STACKSTRING_H_
#include <string>
#include <string.h>
#include <assert.h>
#include <iostream>


/**
 * A String which is living on predefined char* data. (usually on the stack)
 * 
 * It doesn't have any integrity checks (only asserts when NDEBUG is not set), so check yourself what you are doing
 * Note also that the string is not nullterminated.
 * 
 * All optimization is for speeding up the logic unify process
 */
class SString {
public:
	SString (){
		m_data = 0;
		m_length = 0;
		m_maxLength = 0;
	}
	
	SString (char* data, int maxLength, int length = 0)
	: m_maxLength(maxLength), m_length (length), m_data(data){
		assert (maxLength >= 0);
		assert (length >= 0);
	}
	
	/// returns current length
	int length () const { return m_length; }
	
	/// returns the maximum length
	int maxLength () const { return m_maxLength; }
	
	/// returns a substring (note, it doesn't generate a copy, it returns the same memory area)
	SString substr (unsigned int begin, unsigned int length) const {
		return SString (m_data + begin, m_maxLength - begin, length);
	}
	
	/// set to a given string (must be 0 terminated)
	void setTo (const char* text){
		int l;
		for (l = 0; text[l]!=0; l++){
			m_data[l] = text[l];
		}
		m_length = l;
		assert (m_length <= m_maxLength);
	}
	
	/// sets to another string (doens't have to be 0 terminated)
	void setTo (const char* text, int length){
		assert (length <= m_maxLength);
		memmove (m_data, text, length);
		m_length = length;
	}
	
	/// returns the data (not 0 terminated)
	const char * getData() const { return m_data; }
	
	/// set to a given string (c++)
	void setTo (const std::string & text){
		int l;
		for (l = 0; text[l]!=0; l++){
			m_data[l] = text[l];
		}
		m_length = l;
		assert (m_length <= m_maxLength);
	}
	
	/// comparison operator with another string
	bool operator== (const SString & other) const {
		if (m_length != other.length()) return false;
		for (int i = 0; i < other.length(); i++){
			if (m_data[i] != other[i]) return false;
		}
		return true;
	}
	
	/// comparison operator with anoter c++ string
	bool operator== (const std::string & other) const {
		int l = 0;
		for (std::string::const_iterator i = other.begin(); i!=other.end(); i++){
			if (l >= m_length || *i != m_data[l]) return false;
			l++;
		}
		if (l < m_length) return false;
		return true;
	}
	
	/// unequal operator
	bool operator!= (const SString & other) const { return !(*this == other); }
	/// unequal operator against std::string
	bool operator!= (const std::string & other) const { return !(*this == other); }
	
	/// convert it to a C++ string
	operator std::string () const {
		std::string result;
		result.resize(m_length);
		memcpy ((void*)result.c_str(), m_data, m_length);
		return result;
	}
	
	/// r/w access to the characters
	char& operator[] (int i) { return m_data[i]; }
	
	/// r only access to the characters
	const char& operator[] (int i) const { return m_data[i]; }
	
private:
	int m_maxLength;
	int m_length;
	char * m_data;
};

/// output operator
inline std::ostream & operator<< (std::ostream & o, const SString & s) {
	std::string cpps (s);
	return o << cpps;
}

/*
 * Generates a SString on the stack
 * ... SAVENAME ... name of the car[length] field
 * ... NAME     ... name of the resulting SString
 * ... LENGTH   ... length of the field
 */
#define STACKSTRING(SAVENAME, NAME,LENGTH) char SAVENAME [LENGTH]; SString NAME (SAVENAME, LENGTH, 0);

#endif /* STACKSTRING_H_ */
