#!/bin/bash
CSV_FILE=$1 
TABLE_NAME=`echo ${CSV_FILE} | cut -f1 -d_`
TMP_FILE=".tmp_sql_${CSV_FILE}.sql"
PDIR=`pwd`
./create_table.sh $1 > ${TMP_FILE}
echo "LOAD DATA LOCAL INFILE '${PDIR}/${CSV_FILE}' INTO TABLE ${TABLE_NAME} COLUMNS TERMINATED BY ',' LINES TERMINATED BY '\n' IGNORE 1 LINES;" >> ${TMP_FILE}

mysql --local-infile -uroot -pthkbK2bh 'best_first_bandit' < ${TMP_FILE}

rm ${TMP_FILE} 

