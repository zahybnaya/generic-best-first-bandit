#!/bin/bash 
# get hosts
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
