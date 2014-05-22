/*
 * tools.h
 *
 *  Created on: 26.11.2008
 *      Author: nos
 */

#ifndef TOOLS_H_
#define TOOLS_H_


// Some tool functions where I did not know where to put them ;)

#include "logid.h"
#include "microtimer.h"
#include <stdlib.h>
#include <vector>

/// deletes all elements in a vector
template<class T> void deleteElements (typename std::vector<T> & vec){
	for (typename std::vector<T>::iterator i = vec.begin(); i!=vec.end(); i++){
		delete *i;
	}
	vec.clear ();
}

/// generate random number 0 <= value < maxValue
/// see also man rand (3)
inline int maxRand (int maxValue){
	return (int) ((double) (maxValue) * rand () / (RAND_MAX + 1.0));
}

/// generates a string containing empty signs for 2*depth
std::string formatDepth (unsigned int depth);

#endif /* TOOLS_H_ */
