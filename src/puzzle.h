/**
 * Author: Jack Robbins
 * This header file contains multiple reused functions for dealing with puzzles, as well as
 * struct definitions for said puzzles
 */


#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdlib.h>
#include <stdio.h>

/**
* Defines a type of state, which is a structure, that represents a configuration in the gem puzzle game
*/
struct state {
   //Define a dynamic 2D array for the tiles since we have a variable puzzle size
   short** tiles;
   //For A*, define the total_cost, how far the tile has traveled, and heuristic cost int total_cost, current_travel, heuristic_cost;
   int total_cost, current_travel, heuristic_cost;
   //location (row and colum) of blank tile 0
   short zero_row, zero_column;
   //The next state in the linked list(fringe or closed), NOT a successor
   struct state* next;
   //The predecessor of the current state, used for tracing back a solution	
   struct state* predecessor;			
};


//The starting and goal states
struct state* start_state;
struct state* goal_state;
//The fringe is the set of all states open for exploration. It is maintained as a linked list
struct state* fringe = NULL;
//Closed is an array containing all sets previously examined. This is used to avoid repeating
struct state** closed;
//Define an initial starting size of 5000
int closed_max_size = 5000;


/**
 * The initialize_state function takes in a pointer to a state and reserves the appropriate space for the dynamic array
 * that holds the tiles 
 */
void initialize_state(struct state* statePtr, int* N){
	//Declare all of the pointers needed for each row
	statePtr->tiles = (short**)malloc(sizeof(short*) * (*N));

	//For each row, allocate space for N integers
	for(int i = 0; i < (*N); i++){
		statePtr->tiles[i] = (short*)malloc(sizeof(short) * (*N));
	}
}


/**
 * The destroy_state function does the exact reverse of the initialize_state function to properly free memory
 */
void destroy_state(struct state* statePtr, int* N){
	//Go through row by row, freeing each one
	for(int i = 0; i < (*N); i++){
		free(statePtr->tiles[i]);
	}

	//Once all rows are free, free the array of row pointers
	free(statePtr->tiles);
}


/**
 * Prints out a state by printing out the positions in the 4x4 grid
 */
void print_state(struct state* statePtr, int* N){
	//Go through tile by tile and print out
	for (int i = 0; i < (*N); i++) {
		for (int j = 0; j < (*N); j++){
			//Support printing of states with 2 or 3 digit max integers
			if((*N) < 11){	
				//With numbers less than 11, N^2 is at most 99, so only 2 digits needed
				printf("%2d ", statePtr->tiles[i][j]);
			} else {
				//Ensures printing of large states will not be botched
				printf("%3d ", statePtr->tiles[i][j]);
			}
		}
		//Print a newline to represent the row change
		printf("\n");
	}
	printf("\n");
}


/**
 * Performs a "deep copy" from the predecessor to the successor
 */
void copy_state(struct state* predecessor, struct state* successor, int* N){
	//Copy over the tiles array
	for(int i = 0; i < (*N); i++){
		for(int j = 0; j < (*N); j++){
			//Copy tile by tile
			successor->tiles[i][j] = predecessor->tiles[i][j];
		}
	}

	//Initialize the current travel to the predecessor travel + 1
	successor->current_travel = predecessor->current_travel+1;
	//Copy the zero row and column position
	successor->zero_row = predecessor->zero_row;
	successor->zero_column = predecessor->zero_column;
	//Initialize the successor's next to be null
	successor->next = NULL;
	//Set the successors predecessor
	successor->predecessor = predecessor;
}


/**
 * The initialization function takes in the command line arguments and translates them into the initial
 * state. It also initializes the goal state mathematically, as it is always the same
 * Note: Assumes a correct number of command line arguments(16 numbers), must be checked by caller
 */
void initialize_start_goal(char** argv, int* N){
	/* Begin by creating the start state */

	//Create the start state itself
	start_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate memory needed in the start_state
	initialize_state(start_state, N);

	//Start at 1, argv[0] is program name and argv has been adjusted up by 1 to only contain the start state information
	int index = 1;
	short tile;

	//Insert everything into the tiles matrix
	for (int i = 0; i < (*N); i++){
		for (int j = 0; j < (*N); j++){
			//Grab the specific tile number from the arguments and place it into the start state
			tile=atoi(argv[index++]);
			start_state->tiles[i][j] = tile;

			//If we found the zero tile, update the zero row and column
			if(tile == 0){
				start_state->zero_row = i;
				start_state->zero_column = j;
			}
		}
	}

	//Initialize everything else in the start state
	start_state->total_cost = 0;
	start_state->current_travel = 0;
	start_state->heuristic_cost = 0;
	start_state->next = NULL;
	//Important -- must have no predecessor(root of search tree)
	start_state->predecessor = NULL;

	//Print to the console for the user
	printf("\nInitial state\n");
	print_state(start_state, N);


	/* Now we create the goal state */	
	
	//Create the goal state itself
	goal_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate the memory needed in the goal_state
	initialize_state(goal_state, N);	

	int row, col;
	//To create the goal state, place the numbers 1-15 in the appropriate locations
	for(short num = 1; num < (*N) * (*N); num++){
		//We can mathematically find row and column positions for inorder numbers
		row = (num - 1) / (*N);
		col = (num - 1) % (*N);
		goal_state->tiles[row][col] = num;
	}

	//0 is always at the last spot in the goal state
	goal_state->tiles[(*N)-1][(*N)-1] = 0;

	//Initialize everything else in the goal state
	goal_state->zero_row = goal_state->zero_column = (*N)-1;
	goal_state->total_cost = 0;
	goal_state->current_travel = 0;
	goal_state->heuristic_cost = 0;
	goal_state->next=NULL;

	//Print to the console for the user
	printf("Goal state\n");
	print_state(goal_state, N);
}



#endif /* PUZZLE_H */


