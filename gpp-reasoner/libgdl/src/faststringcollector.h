/*
 * faststringcollector.h
 *
 *  Created on: 31.03.2009
 *      Author: nos
 */

#ifndef FASTSTRINGCOLLECTOR_H_
#define FASTSTRINGCOLLECTOR_H_
#include "stackstring.h"
#include <string>
/**
 * Constructing strings via std::ostream is not always fast enough, FastStringCollector tries to minimalize the costs
 * you can insert std::string pointers and stackstrings. Note all data must be existant until the end.
 * 
 */

class FastStringCollector {
public:
	struct Element {
		SString ss;
		const std::string * cpps;
		bool isStackString;
		int length () const { return isStackString?ss.length() : cpps->length(); }
	};
	
	FastStringCollector (){
		data.reserve (16);
	}
	
	/// inserts a stackstring
	void insert (const SString & ss){
		Element e;
		e.ss = ss;
		e.isStackString = true;
		data.push_back(e);
	}
	
	/// inserts a c++ string
	void insert (const std::string & s){
		Element e;
		e.cpps = &s;
		e.isStackString = false;
		data.push_back (e);
	}
	
	std::string str () const {
		// std::cout << LOGID << "Pointing out " << std::endl;
		int length = 0;
		for (std::vector<Element>::const_iterator i = data.begin(); i != data.end(); i++){
			length+=i->length();
		}
		std::string result;
		result.resize (length);
		char * output = (char*) result.c_str();
		for (std::vector<Element>::const_iterator i = data.begin(); i != data.end(); i++){
			const Element & e = *i;
			int el = e.length();
			if (e.isStackString){
				// std::cout << "SS(\"" << e.ss << "\") + ";
				memcpy (output, e.ss.getData(), el);
			} else {
				// std::cout << "Cpp(\"" << (*e.cpps) << "\") + ";
				memcpy (output, (char*) e.cpps->c_str(), el);
			}
			output += el;
		}
		// std::cout << " = " << result << std::endl;
		return result;
	}

private:
	std::vector<Element> data;
};

inline FastStringCollector & operator<< (FastStringCollector & collector, const SString & ss){
	collector.insert (ss);
	return collector;
}

inline FastStringCollector & operator<< (FastStringCollector & collector, const std::string & s){
	collector.insert (s);
	return collector;
}

#endif /* FASTSTRINGCOLLECTOR_H_ */
