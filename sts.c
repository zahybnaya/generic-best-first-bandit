#include "type.h"

void backprop_sts(void *void_ts, treeNode *node, double rollout, int generated, int typeId, int threshold) {
  type_system *ts = void_ts;
  int aboveType = false;
  int i;
  
  while (node != NULL) {
    node->n++;
    if (generated)
      node->subtreeSize++;
    
    if (!aboveType || !TYPE_FROM_ROOT) {
      node->scoreSum += rollout;
    
      if (node->type == true) {
	//Split
	if (node->subtreeSize > threshold) {	  
	  node->type = false;
	  
	  for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	    if (node->children[i]) {
	      node->children[i]->type = true;
	      
	      if (!TYPE_FROM_ROOT) {
		if (typeId != -1) {
		  //reuse the index of the old type
		  ts->types[typeId] = node->children[i];
		  ts->birthdays[typeId] = ts->visits;
		  typeId = -1;
		} else {
		  //need to increase the size of the type system
		  ts->numTypes++;
		  ts->types = realloc(ts->types, ts->numTypes * sizeof(treeNode *));
		  ts->birthdays = realloc(ts->birthdays, ts->numTypes * sizeof(int));
		  
		  ts->types[ts->numTypes - 1] = node->children[i];
		  ts->birthdays[ts->numTypes - 1] = ts->visits;
		}
	      }
	    }
	  }
	}
	
	//the node above the type is treated as a leaf in a minmax tree, thus we want to update the one above him.
	if (node->parent != NULL && node->subtreeSize <= threshold) {
	  //Add action cost for mdps
	  if (_DOM->dom_name == SAILING)
	    rollout += actionCostFindMove(node);
    
	  node = node->parent;
	  node->n++;
	  node->scoreSum += rollout;
	  if (generated)
	    node->subtreeSize++;
	}
	
	aboveType = true;
      }
      
    } else {
      double bestScore = (node->side == max) ? -INF : INF;
      double score;
      
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
	if (node->children[i] && node->children[i]->type == false) { // if child exists, is it the best scoring child?
	  score = node->children[i]->scoreSum / (double)node->children[i]->n;
	  if (((node->side == max) && (score > bestScore)) || ((node->side == min) && (score < bestScore)))
	    bestScore = score;
	}
      }

      node->scoreSum = (node->n) * bestScore; // reset score to that of min/max of children
    }
    
    //Add action cost for mdps
    if (_DOM->dom_name == SAILING)
      rollout += actionCostFindMove(node);

    node = node->parent;
  }
}