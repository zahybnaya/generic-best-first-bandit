#include "type.h"

void backprop_cits(void *void_ts, treeNode *node, double rollout, int generated, int typeId, int threshold) {
  //type_system *ts = void_ts;
  int aboveType = false;
  //int i;

  while (node != NULL) {
    node->n++;
    node->scoreSum += rollout;
    if (generated)
      node->subtreeSize++;
    
    if (aboveType) {
      
      
      
    } else {
      /** NOT GOOD, if i go from below up and check CI ill eventually get that the whole damn tree is types.
       another problem (maybe) is that CI tells you that that is the mean, without caring for noise! its the mean even with the noise ofcourse.*/
      
      if (node->type == true) {
      
	aboveType = true;
      }
    }
    
    //Add action cost for mdps
    if (_DOM->dom_name == SAILING)
      rollout += actionCostFindMove(node);

    node = node->parent;
  }
}