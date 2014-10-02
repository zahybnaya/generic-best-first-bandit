
How to store data. 
----------------

Experiments are reported on CSV files (comma separated).
The first line describes the fields. 

Initially, the 'create_database.sql' script is called 
and creates a database called 'best-first-bandit'

'import.sh' receives a single argument which is the .csv file and creates a table on the mysql if it does not exist. Then, the data on the .csv file is loaded into that table. 

.csv files are named according to the following pattern: <TABLE_NAME>_<WHATEVER>.csv 


The do_all_imports.sh goes over all .csv files and imports everything. 







