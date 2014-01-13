#!/bin/bash 

#######################################################################
#
# Exection script -  (1) See if the commands make sense (2) uncomment the 'eval' lines
#
#
###################################################################

ITERATIONS=(1000 5000 10000 15000 100000)
SIZE_OF_STS=(100 500 700 1000 5000) 
H=(3)
C=('0.5' '2.5')
DOMAIN='0'
WORKING_DIR='.'
EXECUTABLE='./ggames'
SEED='0'
BANDIT_POLICY=(1 2 3)
NUM_GAMES='100'
TYPE_SYS='2'
VERBOSE=true

#start here 
cd ${WORKING_DIR}
if [[ ! -a "${EXECUTABLE}" ]]
then 
	echo "Cannot find ${EXECUTABLE}"
	exit
fi

# Second set: each against MM
for c_ITERATIONS in ${ITERATIONS[@]}
do 
	for c_C in ${C[@]}
	do 
		for c_H in ${H[@]}
		do 
			for c_SIZE_OF_STS in ${SIZE_OF_STS[@]}
			do 
				for c_P in ${BANDIT_POLICY[@]}
				do
				  CMD="${EXECUTABLE} ${DOMAIN} b m -d2 3 -h1 ${c_H} -h2 ${c_H} -c1 ${c_C} -i1 ${c_ITERATIONS} -s ${SEED} -g ${NUM_GAMES} -ts1 ${TYPE_SYS} -t1 ${c_SIZE_OF_STS} -p1 ${c_P}"
				  OUT_FILE="bfbVSmm_${c_H}_${c_C}_${c_P}_${c_ITERATIONS}_${SEED}_${NUM_GAMES}_${TYPE_SYS}_${c_SIZE_OF_STS}.csv"
				  if $VERBOSE ; then 
					echo "${CMD} > ${OUT_FILE}" 
				  fi
				  #eval ${CMD} > ${OUT_FILE}
				done
			done 				
			CMD="${EXECUTABLE} ${DOMAIN} u m -d2 3 -h1 ${c_H} -h2 ${c_H} -c1 ${c_C} -i1 ${c_ITERATIONS} -s ${SEED} -g ${NUM_GAMES}"
			OUT_FILE="uctVSmm_${c_H}_${c_C}_${c_ITERATIONS}_${SEED}_${NUM_GAMES}.csv"

			if $VERBOSE ; then 
				echo "${CMD} > ${OUT_FILE}" 
			fi
			#eval ${CMD} > ${OUT_FILE}
		done 
	done 
done


