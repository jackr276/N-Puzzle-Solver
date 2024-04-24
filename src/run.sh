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
gcc -Wall -Wextra -pthread solve_multi_threaded.c -o solve_multi_threaded

#Get the complexity of the puzzle from the user
read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY
#Get the multithreaded option from user
read -p "Do you want to use multithreading[Y/n]: " MULTITHREADED


#Grab the input(last line of gen output)
input=$(./gen $COMPLEXITY | tail -n 1)

#Use the appropriate version of the program depending on what the user inputted
if [[ $MULTITHREADED == "Y" ]] || [[ $MULTITHREADED == "y" ]]; then
	./solve_multi_threaded $input 
else
	./solve $input
fi
