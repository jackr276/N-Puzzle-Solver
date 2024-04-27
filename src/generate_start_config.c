/**
 * Author: Jack Robbins
 * This simple program generates a 15 puzzle problem configuration that has been "messed up" according to the inputted
 * specification. Takes in the N number of rows and columns, and the amount of random moves that the user wishes to 
 * make. In theory -> more moves = more complex and harder to solve starting state
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//For random move generation
#include <time.h>

//Gem puzzle is 4 rows by 4 columns by default, but this will be changed by user input
int N = 4; 


//Define a simplified state struct for the generation process. We only need the tiles and zero_row and column
struct simplified_state{
	//Define the tiles as an N by N array
	int** tiles;
	short zero_row, zero_column;
};


/**
 * A simple functions that swaps two tiles in the provided state
 * Note: The swap function assumes all row positions are valid, this must be checked by the caller
 */
void swap(int row1,int column1,int row2,int column2, struct simplified_state* statePtr){
	//Store the first tile in a temp variable
	int tile = statePtr->tiles[row1][column1];
	//Put the tile from row2, column2 into row1, column1
	statePtr->tiles[row1][column1] = statePtr->tiles[row2][column2];
	//Put the temp in row2, column2
	statePtr->tiles[row2][column2] = tile;
}


/**
 * Move the 0 slider down by 1 row
 */
void move_down(struct simplified_state* statePtr){
	//Utilize the swap function to move the zero row down by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row+1, statePtr->zero_column, statePtr); 
	//Increment the zero row to keep the position accurate
	statePtr->zero_row++;
}


/**
 * Move the 0 slider right by 1 column
 */
void move_right(struct simplified_state* statePtr){
	//Utilize the swap function to move the zero column right by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row, statePtr->zero_column+1, statePtr); 
	//Increment the zero_column to keep the position accurate
	statePtr->zero_column++;
}


/**
 * Move the 0 slider up by 1 row
 */
void move_up(struct simplified_state* statePtr){
	//Utilize the swap function to move the zero row up by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row-1, statePtr->zero_column, statePtr); 
	//Decrement the zero_row to keep the position accurate
	statePtr->zero_row--;
}


/**
 * Move the 0 left by 1 column
 */
void move_left(struct simplified_state* statePtr){
	//Utilize the swap function to move the zero_column left by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row, statePtr->zero_column-1, statePtr); 
	//Decrement the zero_column to keep the position accurate
	statePtr->zero_column--;
}


/**
 * Prints out a state either in matrix format(option 0) or single row format(option 1)
 */
void print_state(struct simplified_state* statePtr, int format) {
	//Go through each tile in the state, printing out its value 
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++){
			//Print the tile to the console
			printf("%2d ", statePtr->tiles[i][j]);
		}
		//If the option is for matrix format, put a newline after every row
		if(format==0){
			printf("\n");
		}
	}

	//For formatting
	printf("\n");
}


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
	int num_moves;
	//If scanning into a decimnal doesn't work, we know something went wrong
	if(sscanf(argv[1], "%d", &N) != 1 || sscanf(argv[2], "%d", &num_moves) != 1){
		//Print an error and exit
		printf("\nIncorrect type of program arguments.\n");
		printf("Usage: ./generate_start_config <n>\nWhere <r> is number of rows/columns <n> is a positive integer.\n\n");
		exit(1);
	}

	//Create the simplified state that we will use for generation
	struct simplified_state* statePtr = (struct simplified_state*)malloc(sizeof(struct simplified_state));
	//Allocate the space for each of the rows
	statePtr->tiles = malloc(sizeof(int*)*N);
	//Allocate the space for each row
	for(int i = 0; i < N; i++){
		statePtr->tiles[i] = malloc(N*sizeof(int));
	}

	int row, col;
	//Now generate the goal state. Once we create the goal state, we will "mess it up" according to the input number
	for(int index=1; index < N*N; index++){
		//Mathematically generate row position for goal by integer dividing the number by N
		row = (index-1) / N;
		//Mathematically generate column position for goal by finding remainder of row division
		col = (index-1) % N;
		//Put the index in the correct position
		statePtr->tiles[row][col]=index;
	}
	
	//Now that we have generated and placed numbers 1-15, we will put the 0 slider in the very last slot
	statePtr->tiles[N-1][N-1] = 0;
	//Initialize the zero_row and zero_column position for use later
	statePtr->zero_row = N-1;
	statePtr->zero_column = N-1;

	//Set the seed for our random number generation
	srand(time(NULL));

	//Counter for while loop
	int i=0;

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
	print_state(statePtr, 0);

	//Print the initial state for actual use in solve.c
	printf("Initial state in one line, for use in solve.c:\n");
	print_state(statePtr, 1);

	return 0;
}
