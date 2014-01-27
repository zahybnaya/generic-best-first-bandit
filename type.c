#include "type.h"

void *init_type_system(int t, rep_t rep, int side) {
  type_system *ts = calloc(1, sizeof(type_system));
  
  switch (t) {
    case MM_ORACLE:     
      ts->name = MM_ORACLE;
      ts->assignToType = assignToType_mmOracle;
      ts->selectFromType = selectFromType_mmOracle;
      ts->destroy = destroy_mmOracle;
      
      ts->numTypes = MAX_WINS * 2 + 1; //Number of possible minmax values
      ts->types = calloc(ts->numTypes, sizeof(type_mmOracle *));
      
      int i;
      for (i = 0; i < ts->numTypes; i++) {
	ts->types[i] = calloc(1, sizeof(type_mmOracle));
	((type_mmOracle *)ts->types[i])->empty = -1;
      }
      
      ts->extra = calloc(1, sizeof(int));
      *(int *)(ts->extra) = open(ORACLE_PATH, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
      storeMinimax(*(int *)(ts->extra), rep, 0, ORACLE_DEPTH, side, ORACLE_H, 0);
      break;
      
    case STS:
      ts->name = STS;
      ts->assignToType = assignToType_sts;
      ts->selectFromType = selectFromType_sts;
      ts->destroy = destroy_sts;
      ts->numTypes = 1;
      ts->types = calloc(ts->numTypes, sizeof(type_sts *));
      ts->types[0] = calloc(1, sizeof(type_sts));
      break;
      
    case VTS:
      ts->name = VTS;
      ts->assignToType = assignToType_vts;
      ts->selectFromType = selectFromType_vts;
      ts->destroy = destroy_vts;
      
      ts->numTypes = 1;
      ts->types = calloc(ts->numTypes, sizeof(type_vts *));
      ts->types[0] = calloc(1, sizeof(type_vts));
      break;
  }
  return ts;
}
