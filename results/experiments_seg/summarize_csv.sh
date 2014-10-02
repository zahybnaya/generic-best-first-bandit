#!/bin/bash 
#
#  Aggregate all csv's to files
#
#
RESULT_FILE='../seg_mancala_data.csv'
echo "iterations,a1,a2,seg,domain,max_alg, min_alg, num_of_games,num_of_draws,num_of_incomplete,max_win,min_win,total_win_games,max_time,min_time,score,unknown" > ${RESULT_FILE}
grep '^[0-9],' *.csv | sed 's/K/000,/g' | sed 's/9VS2/Trails,DP/g' | sed 's/6VS2/CDP,DP/g' | sed 's/1VS2/MC,DP/g' | sed 's/9VS1/Trails,MC/g' | sed 's/6VS1/CDP,MC/g' | sed 's/B/,/g' | sed 's/:/,/g' | sed 's/ /,/g' | sed 's/.csv//g' >> ${RESULT_FILE}





FIELDS=`cat ${RESULT_FILE} | awk 'BEGIN {FS=","} {print NF}' | sort | uniq | wc -l`
if (( ${FIELDS} > 1)); then 
	echo "ERROR! Problem genereting summary file, lines have different fields. some data is missing. Please check the file manually!"
fi


