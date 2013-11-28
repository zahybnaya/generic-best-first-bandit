
#include "common.h"

/* Return time elapsed since start time */
double getElapsed(Timer start) {
  Timer t;
  gettimeofday(&t, NULL);
  return t.tv_sec - start.tv_sec + (t.tv_usec - start.tv_usec) / 1e6;
}


/* Return the current time */
Timer startTiming(void) {
  Timer t;
  gettimeofday(&t, NULL);
  return t;
}


/* Generates a high quality random seed from OS noise */
unsigned int devrand(void) {
  int fn;
  unsigned int r;
  fn = open("/dev/urandom", O_RDONLY);
  if (fn == -1) {
    puts("No seed");
    exit(-1); /* Failed! */
  }

  if (read(fn, &r, 4) != 4) {
    puts("No seed");
    exit(-1); /* Failed! */
  }

  close(fn);
  return r;
}


/* Swaps the integers in the locations pointed to by the arguments */
void swapInts(int* val1, int* val2) {
  int temp;
  temp = *val1;
  *val1 = *val2;
  *val2 = temp;
}


/* Swaps the pointerss in the locations pointed to by the arguments */
void swapPtrs(void** val1, void** val2) {
  void* temp;
  temp = *val1;
  *val1 = *val2;
  *val2 = temp;
}


/* Swaps the doubles in the locations pointed to by the arguments */
void swapDbls(double* val1, double* val2) {
  double temp;
  temp = *val1;
  *val1 = *val2;
  *val2 = temp;
}
