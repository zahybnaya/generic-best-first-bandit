#define SIMPLE_REGRET_UCT 0 //set to 1 to enable a different algorithm (see minimizing simple regret in MCTS)
#define z975 1.96

extern DOM* _DOM; 
extern int debuglog;
extern int logIteration;
extern int log_this_iteration;

/* This is a node in the UCT tree. */
typedef struct node {
	double scoreSum; // stores the value of this node times n. didnt change the name in order not to break old code.
	int n; // tracks the visit count
	double M2; // variance of rewards == M2 / (n -1)
	double ci; //confidence interval in the score
	double realScoreSum; // this is the sum of total rewards gone through this node.
	int id; // used for graph visualization purposes
	rep_t rep; // generic representation of the state
	int side; // side on move at this board position
	int depth;
	struct node** children; /* pointers to the children of this node -- note that index 0 remains
				   unused (which is reserved for the store), so we have consistent move
				   indexing/numbering */
} treeNode;

typedef struct _child{int index; double value; double SD;} child_data; 

void updateStatistics(treeNode *node, double sample); 

//Backup operators
void minmax_backup(treeNode *node);
void weighted_mm_backup(treeNode *node, double ret);
void ci_backup(treeNode *node, double ret, int ci_threshold);
void variance_all_backup(treeNode *node, double ret, int ci_threshold);
void variance_backup(treeNode *node, double ret, int ci_threshold);
void size_backup(treeNode *node, double ret, int ci_threshold);

//Common function
double avgRewards(treeNode *node);
double assignedScore(treeNode *node);