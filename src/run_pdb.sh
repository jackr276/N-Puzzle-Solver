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

#Switch into the pattern_databases directory
cd pattern_databases

gcc -Wall -Wextra -pthread generate_pattern_db.c -o generate_pattern_db
gcc -Wall -Wextra solve_pattern_db.c -o solve_pattern_db

#Get the puzzle size from the user
read  -p "Enter a positive integer for the NxN puzzle size: " SIZE

#Fault input tolerance -- stop user until positive integer is inputted
while  [[ $SIZE -lt 1 ]]; do
	read -p "Enter a positive integer for the NxN puzzle size: " SIZE
done

#If there is no pattern database, we must generate it
if [[ ! -f "${SIZE}.patterndb" ]]; then
	echo "Pattern database $SIZE is nonexistent\nNow generating pattern database for ${SIZE}x${SIZE}, this may take a while"
	./generate_pattern_db 4
fi

#Go back up to src for generate start config
cd ..

#Get the complexity of the puzzle from the user
read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY

#Faulty input toleramce -- stop user until positive integer is inputted
while [[ $COMPLEXITY -lt 0 ]]; do
	read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY
done

#Grab the last line of generator input
input=$(./generate_start_config $SIZE $COMPLEXITY | tail -n 1)

cd pattern_databases

#Run the solver
./solve_pattern_db $SIZE "${SIZE}.patterndb" $input

cd ..
