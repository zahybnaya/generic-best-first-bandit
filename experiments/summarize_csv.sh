#!/bin/bash 
#
#  Aggregate all csv's to files
#
#
cd ../raw_data/mancala 
RESULT_FILE='../../experiments/mancala_data.csv'
echo "pits,heuristics,c,policy,iterations,seed,num_games,type_system,sts_size,mm_level,domain, max_alg, min_alg, num_of_games,num_of_draws,num_of_incomplete,max_win,min_win,total_win_games" > ${RESULT_FILE}
grep '^[0-9],' * | sed 's/_/,/g' | sed 's/:/,/g' | awk ' BEGIN {FS=","} {$1=6; print $0}' | sed 's/ /,/g' | sed 's/.csv//g' >> ${RESULT_FILE}






FIELDS=`cat ${RESULT_FILE} | awk 'BEGIN {FS=","} {print NF}' | sort | uniq | wc -l`
if (( ${FIELDS} > 1)); then 
	echo "ERROR! Problem genereting summary file, lines have different fields. some data is missing. Please check the file manually!"
fi


