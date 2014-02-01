#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED
#include "domain.h"


typedef struct Hvals_s *Hvals;

void addH(Hvals h,const uid id, double val);
Hvals createHvals(const uid initialSize, const int reallocSize, const double defaultHval,uid endGame);
Hvals* initHvals();

int getNumOfChildren_synth();
//return BranchingFactor

int estimateTreeSize_synth(int treeSize);
// return pow(bran,treeSize)

int isValidChild_synth(rep_t rep, int side, int move);
// return true

int getGameStatus_synth(rep_t rep);
// if ((int)(rep)== setOfFinal...) //else incomplete

void makeMove_synth(rep_t rep,int * side, int move);
//
rep_t cloneRep_synth(rep_t orig);

double applyHeuristics_synth(heuristics_t h,rep_t rep,int side, int budget);

void destructRep_synth(rep_t rep);

rep_t allocate_synth ();

void generateRandomStart_synth(rep_t rep,int side);

rep_t allocate_synth();

void copy_synth (rep_t src ,rep_t dest);

double h1_synth(rep_t rep, int side, int dummy);

#endif // SYNTH_H_INCLUDED
