#Author: Jack Robbins
#A simple runner script for the puzzle solver
#Fault toreance for inputs has been added -- should be adequately user proof

#!/bin/bash

#Make sure that the files exists
if [[ ! -f ./solve.c ]] || [[ ! -f ./generate_start_config.c ]] || [[ ! -f ./solve_multi_threaded.c ]]; then
	echo "Files not found."
	exit 1;
fi

#Compile with aggressive warnings
gcc -Wall -Wextra generate_start_config.c -o generate_start_config 
gcc -Wall -Wextra solve.c -o solve
#IMPORTANT - must link pthread library when compiling
gcc -Wall -Wextra -pthread solve_multi_threaded.c -o solve_multi_threaded

#Get the puzzle size from user
read -p "Enter a positive integer for the NxN puzzle size: " SIZE

#Faulty input tolerance -- stop user until positive integer is inputted
while [[ $SIZE -lt 1 ]]; do
	read -p "Enter a positive integer for the NxN puzzle size: " SIZE
done

#Get the complexity of the puzzle from the user
read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY

#Faulty input tolerance -- stop user until positive integer is inputted
while [[ $COMPLEXITY -lt 0 ]]; do 
	read -p "Enter a positive integer for complexity of initial configuration: " COMPLEXITY
done

#Get the multithreaded option from user
read -p "Do you want to use multithreading[Y/n]: " MULTITHREADED

#Grab the input(last line of gen output)
input=$(./generate_start_config $SIZE $COMPLEXITY | tail -n 1)

#Use the appropriate version of the program depending on what the user inputted
if [[ $MULTITHREADED == "Y" ]] || [[ $MULTITHREADED == "y" ]]; then
	./solve_multi_threaded $SIZE $input 
else
	./solve $SIZE $input
fi
