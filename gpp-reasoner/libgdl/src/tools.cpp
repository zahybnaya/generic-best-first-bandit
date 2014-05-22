#include "tools.h"

std::string formatDepth (unsigned int depth){
	std::string result;
	result.resize(2 * depth);
	for (unsigned int i = 0; i < depth; i++) { result[2 * i] = ' '; result [2 * i + 1] = ' ';}
	return result;
}

