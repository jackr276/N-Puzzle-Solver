#Author: Jack Robbins
#A simple runner script for the puzzle solver

#!/bin/bash

#Make sure that the files exists
if [[ ! -f ./solve.c ]] || [[ ! -f ./GenGemPuzzle.c ]]; then
	echo "Files not found."
	exit 1;
fi

#Compile with aggressive warnings
gcc -Wall -Wextra GenGemPuzzle.c -o gen
gcc -Wall -Wextra solve.c -o solve

#Get the complexity of the puzzle from the user
read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY

#Grab the input(last line of gen output)
input=$(./gen $COMPLEXITY | tail -n 1)

#Run the solver on the input
./solve $input

