#!/bin/bash 
#
#  Aggregate all csv's to files
#
#
cd ../raw_data/6_pits
echo "pits,heuristics,c,policy,iterations,seed,num_games,type_system,sts_size,domain, max_alg, min_alg ,num_of_games,num_of_draws,num_of_incomplete,max_win,min_win,total_win_games" > ../mancala.csv
grep '^[0-9],' * | sed 's/_/,/g' | sed 's/:/,/g' | awk 'FS="," {$1=6; print $0}' | sed 's/ /,/g' | sed 's/.csv//g' >> ../mancala.csv
wc ../mancala.csv

cd ../4_pits
grep '^[0-9],' * | sed 's/_/,/g' | sed 's/:/,/g' | awk 'FS="," {$1=4; print $0}' | sed 's/ /,/g' | sed 's/.csv//g' >> ../mancala.csv
wc ../mancala.csv


