/**
 * Author: Jack Robbins
 * This program generates pattern databases for the N puzzle
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

//Define a global variable for the value of N
int N;

//Define a simplified state struct for the generation process. We only need the tiles and zero_row and column
struct simplified_state{
	//Define the tiles as an N by N array
	int** tiles;
	//Keep track of the zero_row and zero_column for convenience
	short zero_row, zero_column;
	//Enhancement -- keep track of the last move to avoid undoing moves
	int lastMove;
};


struct pattern_cost{
	struct simplified_state* state;
	int* pattern;
	int cost;
	struct pattern_cost* next;
};




//We will keep a linked list of all patterns seen to avoid storing repeats
struct pattern_cost* patterns_first_half = NULL;
struct pattern_cost* patterns_last_half = NULL;
//Keep track of the number of unique patterns generated
int num_unique_patterns = 0;

pthread_mutex_t first_half_mutex;
pthread_mutex_t last_half_mutex;


/**
 * Simple goal state generator. Mathematically generates a goal state and handles all dynamic memory
 * allocation inside the state
 *
 * NOTE: assumes that "initial" has been malloced
 */
void create_goal_state(struct simplified_state* initial){
	//Reserve space for the array of pointers
	initial->tiles = malloc(sizeof(int*)*N);

	//For each pointer(row), reserve space
	for(int i = 0; i < N; i++){
		initial->tiles[i] = malloc(sizeof(int) * N);
	}

	//Mathematically generate the needed numbers -- except for the very last one
	int row, col;
	for(int i = 0; i < N * N - 1; i++){
		row = i / N;
		col = i % N;
		initial->tiles[row][col] = i+1;
	}

	//The last number is always the 0 tile
	initial->tiles[N-1][N-1] = 0;

	//Zero row and zero column are both N-1
	initial->zero_row = initial->zero_column = N-1;

	//Set this to avoid valse positives
	initial->lastMove = -1;
}


/**
 * Since our states are dynamically allocated, we need a teardown function to destroy them
 */
void destroy_state(struct simplified_state* statePtr){
	//Free all of the column arrays first to avoid memory leaks
	for(int i = 0; i < N; i++){
		free(statePtr->tiles[i]);
	}

	//Now free the pointer array
	free(statePtr->tiles);
	//Free statePtr itself
	free(statePtr);

	//Set to null as warning
	statePtr = NULL;
}


/**
 * Define a simple function that checks if two states are identical for use in contained_in_db
 */
int states_same(struct simplified_state* statePtr1, struct simplified_state* statePtr2){
	//Compare tile by tile
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//If we find any difference, return false
			if(statePtr1->tiles[i][j] != statePtr2->tiles[i][j]){
				return 0;
			}	
		}
	}
	
	//Return 1 because they are the same
	return 1;
}


/**
 * This function stores the pattern in a temporary linked list in memory. For simplicity,
 * we will first store all patterns in memory, and then write this linked list to a database
 * file after everything has been made
 */
int store_pattern(struct pattern_cost* patternPtr, int option){
	struct pattern_cost* cursor;

	if(option == 0){
		cursor = patterns_first_half;	
		//Special case, adding at head
		if(patterns_first_half == NULL){
			patterns_first_half = patternPtr;
			return 0;
		}

	} else {
		cursor = patterns_last_half;
		//special case, adding at head	
		if(patterns_last_half == NULL){
			patterns_last_half = patternPtr;
			return 0;
		}
	}

	//Iterate through the entire linked list
	while(cursor->next != NULL){
		if(states_same(cursor->state, patternPtr->state)){
			//Two options here
			if(patternPtr->cost < cursor->cost){
				//We must always store the lowest possible cost, so change it if we find this
				cursor->cost = patternPtr->cost;
			}
			
			//PatternPtr was a repeat, and is useless
			//Free the statePtr in patternPtr
			destroy_state(patternPtr->state);
			//Free the pointer
			free(patternPtr);
			//Set to NULL as a warning
			patternPtr = NULL;
			//They matched, so return true
			return 1;
		}
	
		//Iterate over linked list
		cursor = cursor->next;
	}

	//If we get here, we've reached the tail and we didn't find a repeat, so store patternPtr
	cursor->next = patternPtr;
	//We've added one more unique pattern
	num_unique_patterns++;
	//If we get here, its unique so return false
	return 0;
}


/**
 * A simple function that swaps two tiles in the provided state
 * Note: The swap function assumes all row positions are valid, this must be checked by the caller
 */
int swap(int row1, int column1, int row2, int column2, struct simplified_state* statePtr){
	//Store the first tile in a temp variable
	int tile = statePtr->tiles[row1][column1];
	//Put the tile from row2, column2 into row1, column1
	statePtr->tiles[row1][column1] = statePtr->tiles[row2][column2];
	//Grab and store swapped tile
	int swapped = statePtr->tiles[row2][column2];
	//Put the temp in row2, column2
	statePtr->tiles[row2][column2] = tile;

	return swapped;
}


/**
 * Move the 0 slider down by 1 row
 */
void move_down(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_row down by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row+1, statePtr->zero_column, statePtr);	
	//Increment the zero_row to keep the position accurate
	statePtr->zero_row++;
}


/**
 * Move the 0 slider right by 1 column
 */
void move_right(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_column right by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row, statePtr->zero_column+1, statePtr);	
	//Increment the zero_column to keep the position accurate
	statePtr->zero_column++;
}


/**
 * Move the 0 slider up by 1 row
 */
void move_up(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_row up by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row-1, statePtr->zero_column, statePtr);	
	//Decrement the zero_row to keep the position accurate
	statePtr->zero_row--;
}


/**
 * Move the 0 slider left by 1 column
 */
void move_left(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_column left by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row , statePtr->zero_column-1, statePtr);	
	//Decrement the zero_column to keep the position accurate
	statePtr->zero_column--;
}


/**
 * Genereate and save the pattern for the first N^2/2 tiles in the state saved in patternPtr
 */
void generate_first_half(struct pattern_cost* patternPtr){
	//Our array will have to hold N*N/2 for the first half of the tiles
	int pattern[N * N / 2];

	int tile;

	//Go through each tile checking if it is part of our pattern
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//Grab tile for convenience
			tile = patternPtr->state->tiles[i][j];
			//We only care about the tile if it is in our range and it isn't 0
			if(tile <= N*N/2 && tile != 0){
				//The tile's position encodes its value, and the value stored encodes its pattern position
				//Example: Tile 1 is usually at position 0, but in the pattern its at 5 so at position 0 we store 5
				pattern[tile - 1] = i * N + j;
			}
		}
	}

	patternPtr->pattern = pattern;
}


/**
 * Genereate and save the pattern for the last N^2/2 - 1 tiles in the state saved in patternPtr
 */
void generate_last_half(struct pattern_cost* patternPtr){
	//Our array will have to hold N*N/2 - 1 for the last half of the tiles
	int pattern[N * N / 2 - 1];

	int tile;

	//Go through each tile checking if it is part of our pattern
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//Grab tile for convenience
			tile = patternPtr->state->tiles[i][j];
			//We only care about the tile if it is in our range and it isn't 0
			if(tile >= N*N/2 + 1 && tile != 0){
				//The tile's position encodes its value, and the value stored encodes its pattern position
				//Example: Tile 9 is usually at position 8, but in the pattern its at 5 so at position 0 we store 5
				pattern[tile - (N*N/2 - 1)] = i * N + j;
			}
		}
	}

	patternPtr->pattern = pattern;
}


void* generator_first_half_worker(void* moves){
	//Grab the max_moves from the parameters
	int max_moves = *((int*)moves);

	//Set the seed for random generation
	srand(time(NULL));
	int random_move;

	//Always start at the goal state and work backwards
	struct simplified_state* start = (struct simplified_state*)malloc(sizeof(struct simplified_state));
	//Generate goal state mathematically
	create_goal_state(start);


		//Create the pattern_cost struct
		struct pattern_cost* pc = (struct pattern_cost*)malloc(sizeof(struct pattern_cost));
		//Point the pattern of pc to the newly made start state
		pc->state = start;
		//We are at the goal state, so no cost yet
		pc->cost = 0;

		//Now to generate a pattern randomly, perform i random moves on start
		for(int moves = 0; moves < max_moves; moves++){
			//Get a random move between 0 and 3
			random_move = rand() % 4;
		
			//Every time we successfully make a random move, we've moved one more away from the goal, so increment cost
			//Move left if possible and random_move is 0
			if(random_move == 0 && start->zero_column > 0){
				if(start->lastMove == 1){
						moves--;
						continue;
					}

					move_left(start);
					
			//		printf("%d %d\n", start->zero_row, start->zero_column);
					start->lastMove = 0;
					pc->cost++;
				}

				if(random_move == 1 && start->zero_column < N-1){
					if(start->lastMove == 0){
						moves--;
						continue;
					}

					move_right(start);
			//		printf("%d %d\n", start->zero_row, start->zero_column);
					start->lastMove = 1;
					pc->cost++;
				} 
			
				if(random_move == 2 && start->zero_row < N-1){
					if(start->lastMove == 3){
						moves--;
						continue;
					}
					move_down(start);
			//		printf("%d %d\n", start->zero_row, start->zero_column);
					start->lastMove = 2;
					pc->cost++;
				}

				if(random_move == 3 && start->zero_row > 0){
					if(start->lastMove == 2){
						moves--;
						continue;
					}
					move_up(start);
			//		printf("%d %d\n", start->zero_row, start->zero_column);
					start->lastMove = 3;
					pc->cost++;
				}	
		}	

		//Save the pattern in memory if it is not a repeat
		pthread_mutex_lock(&first_half_mutex);
		store_pattern(pc, 0);
		pthread_mutex_unlock(&first_half_mutex);

	return NULL;
}


void* generator_last_half_worker(void* moves){
	//Set the seed for random generation
	srand(time(NULL));
	//Grab the max_moves from the parameters
	int max_moves = *((int*)moves);

	//Set the seed for random generation
	srand(time(NULL));
	int random_move;

		//Always start at the goal state and work backwards
		struct simplified_state* start = (struct simplified_state*)malloc(sizeof(struct simplified_state));
		//Generate goal state mathematically
		create_goal_state(start);

		//Create the pattern_cost struct
		struct pattern_cost* pc = malloc(sizeof(struct pattern_cost));
		//Point the pattern of pc to the newly made start state
		pc->state = start;
		//We are at the goal state, so no cost yet
		pc->cost = 0;

		//Now to generate a pattern randomly, perform i random moves on start
		for(int moves = 0; moves < max_moves; moves++){
			//Get a random move between 0 and 3
			random_move = rand() % 4;
		
			//Every time we successfully make a random move, we've moved one more away from the goal, so increment cost
			//Move left if possible and random_move is 0
			if(random_move == 0 && start->zero_column > 0){
				if(start->lastMove == 1){
						moves--;
						continue;
					}

					move_left(start);
					start->lastMove = 0;
					pc->cost++;
				}

				if(random_move == 1 && start->zero_column < N-1){
					if(start->lastMove == 0){
						moves--;
						continue;
					}

					move_right(start);
					start->lastMove = 1;
					pc->cost++;
				} 
			
				if(random_move == 2 && start->zero_row < N-1){
					if(start->lastMove == 3){
						moves--;
						continue;
					}
					move_down(start);
					start->lastMove = 2;
					pc->cost++;
				}

				if(random_move == 3 && start->zero_row > 0){
					if(start->lastMove == 2){
						moves--;
						continue;
					}
					move_up(start);
					start->lastMove = 3;
					pc->cost++;
				}		
		}

		
		pthread_mutex_lock(&last_half_mutex);	
		store_pattern(pc, 1);
		pthread_mutex_unlock(&last_half_mutex);

	return NULL;
}


/**
 * Generate patterns back to a certain traceback_depth
 */
void generate_patterns(int traceback_depth){
	/**
	 * From before, N puzzles of a depth less than 30 seem easy to solve quickly. So, for our generation, we will
	 * generate states with moves starting from 30 and up to the traceback_depth
	 */

	//Store the threads in an array
	pthread_t threadArr[20000];

	for(int max_moves = 10; max_moves < traceback_depth; max_moves++){
			
		for(int i = 0; i < 10000; i++){
			pthread_create(&threadArr[i], NULL, generator_first_half_worker, &max_moves);	
		}

		for(int i = 10000; i < 20000; i++){	
			pthread_create(&threadArr[i], NULL, generator_last_half_worker, &max_moves);
		}


		for(int i = 0; i < 20000; i++){
			pthread_join(threadArr[i], NULL);
		}

		printf("Currently generated %d unique patterns\n", num_unique_patterns);
	}
}


/********************************************************************************************
 * Basic Idea: Store some pattern and how many moves it takes to go from that pattern to goal
 * Encoding Format -> attempt to minimize space usage
 * Idea: there are 16 positions on the board, numbers are 0-15
 * --- NUMBER POSITION -----
 *   	ROW_POSITION . . . .   MOVES_FROM_GOAL 
 * With this, we can identify the 
 *
 * So for Example:
 * 1  2  3  4 
 * 5  6  7  8 
 * 9  10 11 12 
 * 13 14 15 0
 * Can be encoded as
 * 0 1 2 3 4 5 6 7 0 

 *******************************************************************************************/



int main(int argc, char** argv){
	//Check to ensure proper number of arguments
	if(argc < 2){
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle\n");
		return 0;
	}

	//Which group size do we want to generate patterns for
	int group_size;

	//Get the N and G value from the user for which database that they want to generate 
	if(sscanf(argv[1], "%d", &N) != 1){	
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle\n");
		return 0;
	}

	//We won't was time generating databases for 3 puzzles, N must be more than 3 for the computational cost to be worth it
	if(N < 4){
		printf("It is entirely unnecessary to use a pattern database for a 3 X 3 puzzle. Now exiting. \n");
		return 0;
	}


	//Filename is always of format "N_G.patterndb"
	char db_filename[14];

	//Save the filename into a string
	sprintf(db_filename, "%d.patterndb", N);


	pthread_mutex_init(&first_half_mutex, NULL);
	pthread_mutex_init(&last_half_mutex, NULL);
	//Test
	printf("Now generating database for %d puzzle problem\n", N);
	generate_patterns(100);
	printf("Success! Generated %d distinct patterns\n", num_unique_patterns);


	pthread_mutex_destroy(&first_half_mutex);
	pthread_mutex_destroy(&last_half_mutex);

	printf("Saving to database file: %s\n\n", db_filename);
	return 0;
}
