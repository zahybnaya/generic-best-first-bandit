#!/bin/bash 
# get hosts
                                                                                                    
####################################################################################################
# Distributes a task on multiple computers.                                                        #
# Uses Python                                                                                      #
#                                                                                                  #
# 1) Produce an execution file (executions are unique)                                             #
# 2) call the script with <working dir> python distributer.py <execution_file>                     #
# 3) locks are stored at .locks by default                                                         #
# 4) Assumes a list of hosts at "readyServers"                                                     #
#                                                                                                  #
#  Example:                                                                                        #
#                                                                                                  #
# distribute_exections.sh "./test_execution/generic-best-first-bandit" "python distributer.py exe" #
#                                                                                                  #
####################################################################################################



hosts=(`cat readyServers | uniq`)
WORKING_DIR="$1"
EXPERIMENTER_COMMAND="$2"
echo 'Running the following command :' ${EXPERIMENTER_COMMAND}   
echo 'Working dir is :' ${WORKING_DIR}   
echo "Total ${#hosts[@]} hosts"
read -p "Press any key to start..."

# for each hostname
for host in ${hosts[@]}
do
    echo 'Connecting to' ${host}
	ssh -t -t ${host} << END_SSH
	cd ${WORKING_DIR}
	screen -d -m ${EXPERIMENTER_COMMAND}
	exit
END_SSH
done
