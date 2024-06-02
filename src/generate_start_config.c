/**
 * Author: Jack Robbins
 * This simple program generates a 15 puzzle problem configuration that has been "messed up" according to the inputted
 * specification. Takes in the N number of rows and columns, and the amount of random moves that the user wishes to 
 * make. In theory -> more moves = more complex and harder to solve starting state
 *
 */


//For random move generation
#include <time.h>
#include "puzzle.h"


/**
 * The main function handles all input checking, creating the state, and "messing up" the state sufficiently, after which
 * it will print both the matrix version and the one line version of the state to the console
 */
int main(int argc, char** argv) {
	//If there aren't enough program arguments, print an error and exit
	if(argc != 3){
		printf("\nIncorrect number of program arguments.\n");
		printf("Usage: ./generate_start_config <r> <n>\nWhere <r> is number of rows/columns and <n> is initial complexity.\n\n");
		exit(1);
	}

	//Once we know there are the right amount of arguments, attempt to scan the number of moves into numMoves
	int num_moves, N;
	//If scanning into a decimnal doesn't work, we know something went wrong
	if(sscanf(argv[1], "%d", &N) != 1 || sscanf(argv[2], "%d", &num_moves) != 1){
		//Print an error and exit
		printf("\nIncorrect type of program arguments.\n");
		printf("Usage: ./generate_start_config <n>\nWhere <r> is number of rows/columns <n> is a positive integer.\n\n");
		exit(1);
	}

	//Create the simplified state that we will use for generation
	struct state* statePtr = (struct state*)malloc(sizeof(struct state));
	//Iniitialize the state with helper function
	initialize_state(statePtr, N);

	int row, col;
	//Now generate the goal state. Once we create the goal state, we will "mess it up" according to the input number
	for(short index = 1; index < N*N; index++){
		//Mathematically generate row position for goal by integer dividing the number by N
		row = (index-1) / N;
		//Mathematically generate column position for goal by finding remainder of row division
		col = (index-1) % N;
		//Put the index in the correct position
		statePtr->tiles[row][col] = index;
	}
	
	//Now that we have generated and placed numbers 1-15, we will put the 0 slider in the very last slot
	statePtr->tiles[N-1][N-1] = 0;
	//Initialize the zero_row and zero_column position for use later
	statePtr->zero_row = N-1;
	statePtr->zero_column = N-1;

	//Set the seed for our random number generation
	srand(time(NULL));

	//Counter for while loop
	int i = 0;

	//A variable to store our random move numbers in
	int random_move;
	//The main loop of our program. Keep randomly messing up the goal config as many times as specified
	//In theory -- higher number inputted = more complex config
	while(i < num_moves){
		//Get a random number from 0 to 4
		random_move = rand() % 4;	

		//We will keep the same convention as in the solver
		// 0 = left move, 1 = right move, 2 = down move , 3 = up move
		
		//Move left if possible and random_move is 0
		if(random_move == 0 && statePtr->zero_column > 0){
			move_left(statePtr);
		}

		//Move right if possible and random move is 1
		if(random_move == 1 && statePtr->zero_column < N-1){
			move_right(statePtr);
		}

		//Move down if possible and random move is 2
		if(random_move == 2 && statePtr->zero_row < N-1){
			move_down(statePtr);
		}

		//Move up if possible and random move is 3
		if(random_move == 3 && statePtr->zero_row > 0){
			move_up(statePtr);
		}

		//Increment i
		i++;
	}

	//Print the initial state in puzzle(matrix) format
	printf("Initial state in puzzle format:\n");
	print_state(statePtr, N, 0);

	//Print the initial state for actual use in solve.c
	printf("Initial state in one line, for use in solve.c:\n");
	print_state(statePtr, N, 1);

	return 0;
}
