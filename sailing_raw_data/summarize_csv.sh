#!/bin/bash 
#
#  Aggregate all csv's to files
#
#
cd ./sailing_greedy_probTo
RESULT_FILE='../sailing_data_greedy_pathProb.csv'
echo "pits,heuristics,c,iterations,seed,num_games,policy,type_system,sts_size,domain, max_alg, min_alg, num_of_games,num_of_draws,num_of_incomplete,max_win,min_win,total_win_games,max_time,min_time,avg_score" > ${RESULT_FILE}
grep '^[0-9],' * | sed 's/_/,/g' | sed 's/:/,/g' | awk ' BEGIN {FS=","} {$1=10; print $0}' | sed 's/ /,/g' | sed 's/.csv//g' >> ${RESULT_FILE}

FIELDS=`cat ${RESULT_FILE} | awk 'BEGIN {FS=","} {print NF}' | sort | uniq | wc -l`
if (( ${FIELDS} > 1)); then 
	echo "ERROR! Problem genereting summary file, lines have different fields. some data is missing. Please check the file manually!"
fi


