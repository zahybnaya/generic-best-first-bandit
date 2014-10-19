# Runs the eval states proceudre 
#    -g number of games 
#    -t ci minimal threshold 
#    -i number of iterations 
#    -s seed 
#    -h heuristic 
#    -gs gold standard level

#!/bin/bash

rm evalstates
cp ../../evalstates . 
RESULT_FILE="../../results/evalstates/states.b4.c0.csv"
./evalstates -i 10      -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 200     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 300     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 400     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 600     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 700     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 800     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 900     -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 1000    -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 2000    -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 3000    -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 4000    -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 5000    -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 10000   -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100000  -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500000  -h 4 -b 4 -g 50 -c 0.0 >> $RESULT_FILE


RESULT_FILE="../../results/evalstates/states.b8.c0.csv"
./evalstates -i 10      -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 200     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 300     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 400     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 600     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 700     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 800     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 900     -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 1000    -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 2000    -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 3000    -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 4000    -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 5000    -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 10000   -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100000  -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500000  -h 4 -b 8 -g 50 -c 0.0 >> $RESULT_FILE


RESULT_FILE="../../results/evalstates/states.b24.c0.csv"
./evalstates -i 10      -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 200     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 300     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 400     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 600     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 700     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 800     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 900     -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 1000    -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 2000    -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 3000    -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 4000    -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 5000    -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 10000   -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 100000  -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
./evalstates -i 500000  -h 4 -b 24 -g 50 -c 0.0 >> $RESULT_FILE
