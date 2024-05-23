#Author: Jack Robbins
#A simple runner script for the pattern database solver

#!/bin/bash

#If we can't find the files, quit the program
if [[ ! -f ./pattern_databases/generate_pattern_db.c ]] || [[ ! -f ./pattern_databases/solve_pattern_db.c ]]; then
	echo "Files not found."
	exit 1
fi

#Compile all with aggressive warnings
gcc -Wall -Wextra generate_start_config.c -o generate_start_config
gcc -Wall -Wextra -pthread ./pattern_databases/generate_pattern_db.c -o ./pattern_databases/generate_pattern_db
gcc -Wall -Wextra ./pattern_databases/solve_pattern_db.c -o ./pattern_databases/solve_pattern_db

#Get the puzzle size from the user
read  -p "Enter a positive integer for the NxN puzzle size: " SIZE

#Fault input tolerance -- stop user until positive integer is inputted
while  [[ $SIZE -lt 1 ]]; do
	read -p "Enter a positive integer for the NxN puzzle size: " SIZE
done

#If there is no pattern database, we must generate it
if [[ ! -f ./pattern_databases/"${SIZE}.patterndb" ]]; then
	echo "Pattern database $SIZE is nonexistent\nNow generating pattern database for ${SIZE}x${SIZE}, this may take a while"
	./pattern_databases/generate_pattern_db 4
fi

#Get the complexity of the puzzle from the user
read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY

#Faulty input toleramce -- stop user until positive integer is inputted
while [[ $COMPLEXITY -lt 0 ]]; do
	read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY
done

#Grab the last line of generator input
input=$(./generate_start_config $SIZE $COMPLEXITY | tail -n 1)

#Run the solver
./pattern_databases/solve_pattern_db $SIZE "./pattern_databases/${SIZE}.patterndb" $input
