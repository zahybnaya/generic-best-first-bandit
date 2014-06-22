#ifndef GGP_H_INCLUDED
#define GGP_H_INCLUDED
#include "domain.h"

typedef struct rules* rules_t;
typedef struct rules_holder* rules_holder_t; 

rules_t compile();
rules_holder_t createRulesHolder(rules_t);

getNumOfChildren_func generateGetNumOfChildren(rules_holder_t);

#endif // GGP_H_INCLUDED
