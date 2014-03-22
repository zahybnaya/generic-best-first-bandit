#include "type.h"

void assignToType_sts(void *void_ts, treeNode *node, int fatherType, int threshold, int policy) {
  type_system *ts = (type_system *)void_ts;
  
  if (fatherType == -1) { //no father means root node
    ts->types[0]->root = node;
    ts->types[0]->birth = ts->visits;
  } else {
    type *t = ts->types[fatherType];
    
    //The size of a type is the size of the subtree rooted at the node representing the type
    //which is updated at the expansion phase, so only need to take care of splitting types
    if (t->root->subtreeSize > threshold) {
      //In order to stay consistent with the type to insert into all current children, find out who of the children of the root of the type
      //is the ancestor of the current node to be inserted.
      //This ancestor will become the root of a new type which will be in the same index as the old type.
      treeNode *newFather = node;
      while (newFather->parent != t->root)
newFather = newFather->parent;
      
      int i;
      for (i = 1; i < _DOM->getNumOfChildren(); i++) {
//skip empty children or this is the new father (will be taken care of later)
if (!_DOM->isValidChild(t->root->rep, t->root->side, i) || t->root->children[i] == newFather)
continue;

ts->numTypes++;
ts->types = realloc(ts->types, ts->numTypes * sizeof(type *)); //alocate a new type

ts->types[ts->numTypes - 1] = calloc(1, sizeof(type));
ts->types[ts->numTypes - 1]->root = t->root->children[i];
ts->types[ts->numTypes - 1]->birth = ts->visits;

if (policy == DELETE_VMAB)
ts->types[ts->numTypes - 1]->root->n = 0;

      }

      t->root = newFather;
      t->birth = ts->visits;
      if (policy == DELETE_VMAB)
t->root->n = 0;
    }
  }
}