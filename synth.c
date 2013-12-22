#include "synth.h"
#include <math.h>

#define b 3 /*Branching factor*/
#define EPS 0.03 /*Epsilon*/
#define MIN_DEPTH 7 /*Min depth to choose random endgame*/
#define DEPTH_INTERVAL 2 /*Interval to choose random endgame*/
#define END_GAME -1 /* END_GAME for tree (-1 rand)*/
#define RATIO_PRECISON 1000 /* decimal precision of ratio */
#define H_MINIMAX_DEPTH 15 /* depth of mm for filling H */
#define NOISE_MU 90 /* Noise Mu sent to normal noise */
#define NOISE_SD 5 /* Noise Standard Div*/
#define DEFAULT_H_VAL  2 /* default h to use */
#define RAND_LEAVS false



/**
*
*  rep_t in this case is just a number
*
*/


extern int debuglog ;
struct Hvals_s {
    uid* ids;
    double * vals;
    double defaultH;
    uid size;
    uid capacity;
    int reallocSize;
    uid endGameId;
};


static Hvals hvals = NULL;

static void depthIndexOf(const uid id, int* depth, uid* prefix, uid* suffix);
static uid findId(const int depth, const double ratio);
static uid succesorsIds(const uid id, const int k, uid* from, uid* to);
//static int addTrap(const int depth, const double r, const int k, Hvals h);
//static int addTrapById(const uid id, const int k, Hvals h);
//static void addTraps(int howManyTraps,const int sizeofTrap,Hvals h, uid* traps,int trap_gap);
static void freeHvals();
static uid succ(const uid id, const int move);
static void printHvals(Hvals hvals);
static double searchIndexOf(const uid id);
static double getH(const uid id);

/*
* return ID of node by depth and ratio
*/
static uid findId(const int depth, const double ratio){
    int initialsDepth=0;
    uid accum=0;
    uid nextDepthWidth=0;
    while(initialsDepth<depth){
        accum += pow(b,initialsDepth++);
        nextDepthWidth = pow(b,initialsDepth);
    }
    uid ret = accum + (ratio*nextDepthWidth);
    assert(ret>0); //integer overflow
    return ret;
}

/*
* Successor ids by rootid and k
*/
static uid succesorsIds(const uid id, const int k, uid* from, uid* to){
    assert(k>0);
    *from = succ(id, 1);
    if(k>1){
        succesorsIds(*from,k-1, from, to);
    }
    *to=*from+pow(b,k)-1;
    uid ret = (*to)-(*from)+1;
    assert(ret>0);
    return ret;
}

/*
    returns the succesor of id for move move
*/
static uid succ(const uid id, const int move){
    uid pred,succ;
    int depth;
    depthIndexOf(id,&depth,&pred,&succ);
    uid val = id + succ + (pred * b) + move;
    assert(val>0);
    return val;
}

/*
* Create the Hvals with initialSize
* endGame is the end of tree - For a random end (based on MIN_DEPTH and DEPTH_ITERVAL) send negative
*/
Hvals createHvals(const uid initialSize, const int reallocSize, const double defaultHval,uid endGame){
    Hvals h = (Hvals)malloc(sizeof(struct Hvals_s));
    h->size  = 0;
    h->reallocSize = reallocSize;
    h->defaultH = defaultHval;
    h->ids = (uid*)malloc(sizeof(uid)*initialSize);
    h->vals = (double*)malloc(sizeof(double)*initialSize);
    h->capacity = initialSize;
    if(END_GAME<0){
        int depth = MIN_DEPTH + (rand() % DEPTH_INTERVAL);
        assert(H_MINIMAX_DEPTH>depth);
        double ratio = (rand() % RATIO_PRECISON) / ((double)RATIO_PRECISON);
        h->endGameId = findId(depth,ratio);
    }else{
        h->endGameId = endGame;
    }
    return h;
}

/*
* Add heuristic value to the Hvals structure
*/
void addH(Hvals h,const uid id, double val){
    int ind=-1;
    if((ind = searchIndexOf(id))>=0){/*Exists*/
        h->vals[ind] = val;
    }
    /*Append*/
    if(h->size>=h->capacity){
        h->capacity+=h->reallocSize;
        h->ids = (uid*)realloc(h->ids,sizeof(uid)*(h->capacity));
        h->vals = (double*)realloc(h->vals,sizeof(double)*(h->capacity));
    }
    h->ids[h->size] =  id;
    h->vals[h->size] =  val;
    assert(h->size<h->capacity);
    h->size++;
}

/* Get H*/
static double getH(const uid id){
    int ind=-1;
    if((ind = searchIndexOf(id))>=0)
        return hvals->vals[ind];

    return hvals->defaultH;
}

/*
 Search for the index of id
*/
static double searchIndexOf(const uid id){
    uid ind = 0;
    for(;ind<hvals->size;ind++){
        if(hvals->ids[ind]==id){
            return ind;
        }
    }
    return -1;
}
/*
* free the Hvals
*/
static void freeHvals(){
    if(hvals==NULL) return;
    free(hvals->ids);
    free(hvals->vals);
    free(hvals);
}


/*
Add traps
*/
//static void addTraps(int howManyTraps,const int sizeofTrap,Hvals h, uid* traps,int trap_gap){
    //if(!traps){
        //printf("traps array not initialized\n");
        //exit(-1);
    //}
    //uid trapRoot;
    //int i;
    //for(i=0;i<howManyTraps;i++){
        //trapRoot = 2 + (rand() % (h->endGameId+2));
        //if(debuglog)printf("Adding trap AT %d\n", trapRoot);
        //addTrapById(trapRoot,sizeofTrap,h);
        //traps[i] = trapRoot;
    //}
//}

/*

*/
static int addTrapById(const uid id, const int k, Hvals h){
    if(h==NULL){
        printf("H is null\n");
        return 0;
    }
    uid from, to;
    int firstUse = true;
    int d = k-1;
    while(d>0){
        uid elements = succesorsIds(id,d--,&from,&to);
        int i=0;
        double trapVal = firstUse ? MIN_WINS : MAX_WINS-EPS;
        firstUse = false;
        for (i=0;i<elements;i++){
            if(debuglog)printf("HVALS_ADD:%d=%.2f\n",from+i,trapVal);
            addH(h,from+i,trapVal);
        }
    }
    addH(h,id,MAX_WINS-EPS);
    return 1;
}


/*
* add a trap to h
*/
//static int addTrap(const int depth, const double r, const int k, Hvals h){
  //  if(h==NULL){
    //   printf("H is null\n");
    //   return 0;
   // }
   // int root_id = findId(depth,r);
   // return addTrapById(root_id,k,h);
//}



/*

*/
int getNumOfChildren_synth(){
    return b+1;
}

/* Copys from src to dest */
void copy_synth (rep_t src ,rep_t dest){
    *(uid*)dest = *(uid*)src;
}

/*
*
 Inits the hvals
**/
Hvals* initHvals(){
    freeHvals();
    hvals = NULL;
    if(hvals==NULL){
        hvals = createHvals(1000,100,DEFAULT_H_VAL,END_GAME);
    }
    return &hvals;
}



/*
 Generates a random game-tree
*/
void generateRandomStart_synth(rep_t rep,int side){
    initHvals();
    *(uid*)rep=1;
//    mmNode* n =  generateMmNode(rep,side,false);
//    makeNormalNoise(rep,n,H_MINIMAX_DEPTH,NOISE_SD,NOISE_MU,hvals,getNumOfChildren_synth());
    //addTrapById(3,3,hvals);
    //traps[0]=3;
    if(debuglog)printHvals(hvals);
//    addTraps(howManyTraps,sizeofTrap,hvals,traps,trap_gap);
}

/*
*/
int estimateTreeSize_synth(int treeSize){
 return pow(getNumOfChildren_synth()-1,treeSize);
}



void makeMove_synth(rep_t rep,int * side, int move){
    assert(move>0);
    *side = 1^*side;
    uid id= *(uid*)rep;
    uid pred,succ;
    int depth;
    depthIndexOf(id,&depth,&pred,&succ);
    uid val = id + succ + (pred * b) + move;
    *(uid*)rep=val;
}

rep_t cloneRep_synth(rep_t orig){
        uid* ret = (uid*)malloc(sizeof(uid));
        *ret = *(uid*)orig;
        return ret;
}

int getGameStatus_synth(rep_t rep){
    int result = applyHeuristics_synth((heuristics_t)NULL,rep,max,0);
    if(result == MAX_WINS||result == MIN_WINS) {
        return result;
    }
    uid crep = *(uid*)rep;
    if(crep > hvals->endGameId){
            uid winDetermine;
            if(RAND_LEAVS){
                winDetermine = rand();
            }else{
                winDetermine = crep;
            }
            int val = (winDetermine%2==0)?MAX_WINS:MIN_WINS;
            addH(hvals,crep,val);
            return val;
    }

    return INCOMPLETE;
}


int isValidChild_synth(rep_t rep, int side, int move){
    //const int id = *(int*)rep;
    return true;
}




/*
* Apply the heuristics
*/
double applyHeuristics_synth(heuristics_t h,rep_t rep,int side, int budget){
    uid const crep = *(uid*)rep;
    double hVal=0.0;
    hVal = getH(crep);
    if(debuglog)printf("Using h(%d)->H:%.2f\n",crep,hVal);
    return hVal;

//     double hVal=0.0;
//    int depth, before,after;
//    depthIndexOf(crep,&depth,&before,&after);
//    double r = INF;
//    if(after!=0)
//            r = (double)before/after;
//    if(depth==6){
//        if(r>3){
//            hVal = crep%2;
//        }
//        hVal = 1;
//    } else {
//        if(r<1){
//            hVal=1-3*EPS;
//        } else if(r>3){
//            hVal= 1-EPS;
//        }
//        else{
//            hVal= 1-2*EPS;;
//        }
//    }

//     switch(crep){
//        case 14:
//            hVal=MIN_WINS;
//            break;
//        case 15:
//            hVal=MIN_WINS;
//            break;
//        case 12:
//            hVal=MAX_WINS-EPSILON;
//            break;
//        case 13:
//            hVal=MAX_WINS-EPSILON;
//            break;
//        case 4:
//            hVal=MAX_WINS-2*EPSILON;
//            break;
//        case 5:
//            hVal=MAX_WINS-2*EPSILON;
//            break;
//        case 3:
//            hVal=MAX_WINS-EPSILON;
//            break;
//        case 6:
//            hVal=MAX_WINS-EPSILON;
//            break;
//        case 7:
//            hVal=MAX_WINS-0.1*EPSILON;
//            break;
//        default:
//            return hVal=1;
//     }
//
//    return crep;
}


/*
* Returns the depth, # of items before (on that depth) and # items after
*/
static void depthIndexOf(const uid id, int* depth, uid* prefix, uid* suffix){

    if(id==1){
        *prefix=0;
        *suffix=0;
        *depth=0;
        return;
    }
    int depthSearch  = 0;
    uid accum = 1, prev=1;

    while(id > accum){
        prev = accum;
        accum += pow(b,(++depthSearch));
    }
    *depth = depthSearch;
    *prefix =  (id - prev)-1;
    *suffix= (accum - id);
}




rep_t allocate_synth(){
    return malloc(sizeof(uid));
}

void destructRep_synth(rep_t rep){
    free((uid*)rep);
}





//static void test();

static void printHvals(Hvals hvals){
    int i=0;
    printf("*********Dump 0f Hvals**********\n");
    printf("Size: %d, endGameId:%d \n",hvals->size, hvals->endGameId);
    for(i=0;i<hvals->size;i++){
        printf("%d=%.2f\n",hvals->ids[i],hvals->vals[i]);
    }
    printf("*********Dump 0f Hvals**********\n");
    fflush(stdout);
}
//void main(){
//    test();
//}

/*
void test(){

    int depth=0;
    uid pref=0,suffix = 0;
    depthIndexOf(127,&depth,&pref,&suffix);
    depthIndexOf(200,&depth,&pref,&suffix);

    depthIndexOf(6,&depth,&pref,&suffix);
    printf("Depth: %d, pref: %d suffix: %d\n",depth,pref,suffix);
    assert(depth==2);
    assert(pref==2);
    assert(suffix==1);

    depthIndexOf(7,&depth,&pref,&suffix);
    printf("Depth: %d, pref: %d suffix: %d\n",depth,pref,suffix);
    assert(depth==2);
    assert(pref==3);
    assert(suffix==0);

    depthIndexOf(4,&depth,&pref,&suffix);
    printf("Depth: %d, pref: %d suffix: %d\n",depth,pref,suffix);
    assert(depth==2);
    assert(pref==0);
    assert(suffix==3);

    depthIndexOf(2,&depth,&pref,&suffix);
printf("Depth: %d, pref: %d suffix: %d\n",depth,pref,suffix);
    assert(depth==1);
    assert(pref==0);
    assert(suffix==1);


    depthIndexOf(1,&depth,&pref,&suffix);
    printf("Depth: %d, pref: %d suffix: %d\n",depth,pref,suffix);
    assert(depth==0);
    assert(pref==0);
    assert(suffix==0);

    int source = 3, side = max;
    int move = 1;


    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==6);
    assert(side==min);

    source = 1; side = max;
    move = 1;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==2);
    assert(side==min);

    source = 1; side = max;
    move = 2;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==3);
    assert(side==min);


    source = 2; side = min;
    move = 1;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==4);
    assert(side==max);

    source = 2; side = min;
    move = 2;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==5);
    assert(side==max);

    source = 5; side = min;
    move = 1;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==10);
    assert(side==max);

    source = 5; side = min;
    move = 2;
    makeMove_synth(&source,&side,move);
    printf("new Move: %d new Side: %d \n",source,side);
    assert(source==11);
    assert(side==max);

    int m=3;
    assert(getGameStatus_synth(&m)==INCOMPLETE);
    m=8;
    assert(getGameStatus_synth(&m)==MAX_WINS);
    m=15;
    assert(getGameStatus_synth(&m)==MIN_WINS);

    assert(estimateTreeSize_synth(3)==8);

    uid * clone = cloneRep_synth(&m);
    assert(*clone==m);
    *clone=9;
    assert(*clone!=m);

    m = 3;
    heuristics_t dummy=NULL;
    assert(applyHeuristics_synth(dummy,&m,max,2)==1);
    m = 10;
    assert(applyHeuristics_synth(dummy,&m,max,2)==MAX_WINS);

}
*/
