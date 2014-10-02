# Runs the eval states proceudre 
#    -g number of games 
#    -t ci minimal threshold 
#    -i number of iterations 
#    -s seed 
#    -h heuristic 
#    -gs gold standard level

#!/bin/bash

cp ../../evalstates . 
RESULT_FILE="../../results/evalstates/states.csv"
./evalstates -i 10      -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 100     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 200     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 300     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 400     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 500     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 600     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 700     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 800     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 900     -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 1000    -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 2000    -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 3000    -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 4000    -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 5000    -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 10000   -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 100000  -h 3 -g 100 >> $RESULT_FILE
./evalstates -i 500000  -h 3 -g 100 >> $RESULT_FILE
















