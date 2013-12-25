#!/bin/sh
# pass in the file name as an argument: ./create_table.sh filename.csv
CSV_FILE=$1
TABLE_NAME=`echo ${CSV_FILE} | cut -f1 -d_`
echo "CREATE TABLE IF NOT EXISTS ${TABLE_NAME} ( "
head -1 $1 | sed -e 's/,/ VARCHAR(255),\n/g'
echo " VARCHAR(255) );"
