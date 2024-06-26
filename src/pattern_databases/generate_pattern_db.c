/**
 * Author: Jack Robbins
 * This program generates pattern databases for the N puzzle
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


//Define a self referential struct to store a pattern and a cost. The pattern is what we really care about here
struct pattern_cost{
	//We store the pattern as an array using positional encoding -- more info below
	int* pattern;
	//We must also store the pattern length
	int pattern_length;
	//The number of moves that it took to get this pattern
	int cost;
	//0 = first half, 1  = last half
	int pattern_type;
	//For linked list functionality
	struct pattern_cost* next;
};


//A simple data structure that we can pass to our threads
struct thread_params{
	//Maximum number of moves to make
	int max_moves;
	//The type of pattern, 0 = first half, 1 = last half
	int pattern_type;
};


//We will keep a linked list of all patterns seen to avoid storing repeats
struct pattern_cost* patterns_first_half = NULL;
struct pattern_cost* patterns_last_half = NULL;

//Keep track of the number of unique patterns generated
int num_unique_patterns = 0;

//We need two write locks to avoid race conditions
pthread_mutex_t first_half_lock;
pthread_mutex_t last_half_lock;


/**
 * Simple goal state generator. Mathematically generates a goal state and handles all dynamic memory
 * allocation inside the state. If the option is 0, the N^2/2 numbers will be 0 because they're irrelevant
 * to the pattern. If the option is 1, then the exact opposite will be true
 *
 * NOTE: assumes that "initial" has been malloc'd
 */
void create_goal_state(struct simplified_state* initial, int option){
	//Reserve space for the array of pointers
	initial->tiles = malloc(sizeof(int*)*N);

	//For each pointer(row), reserve space
	for(int i = 0; i < N; i++){	
		//Initialize each row with 0s
		initial->tiles[i] = calloc(N, sizeof(int));
	}

	//Mathematically generate the needed numbers -- except for the very last one
	int row, col;

	//0 means generate the first N^2/2 numbers 
	if(!option){
		for(int i = 0; i < N * N / 2; i++){
			row = i / N;
			col = i % N;
			initial->tiles[row][col] = i+1;
		}
	} else {
		//Otherwise, generate the last half of the numbers
		//Note: Last pattern has N^2 - 1 numbers, 0 never counts
		for(int i = N * N / 2; i < N*N; i++){
			row = i / N;
			col = i % N;
			initial->tiles[row][col] = i+1;
		}
	}	

	//The last number is always the 0 tile
	initial->tiles[N-1][N-1] = 0;

	//Zero row and zero column are both N-1
	initial->zero_row = initial->zero_column = N-1;

	//Set this to avoid false positives
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
 * A simple function that prints out a state
 */
void print_state(struct simplified_state* statePtr){
	//Go through each row and column, printing out the tile 
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			printf("%2d ", statePtr->tiles[i][j]);
		}
		printf("\n");
	}
	//For formatting
	printf("\n");
}


/**
 * Define a simple function that checks if two patterns are identical for use in contained_in_db
 */
int patterns_same(int* pattern1, int* pattern2, int length){
	//Compare tile by tile
	for(int i = 0; i < length; i++){
		//If there's one mismatch, return and exit
		if(pattern1[i] != pattern2[i]){
			return 0;
		}	
	}
	
	//Return 1 because they are the same, 1 = true
	return 1;
}


/**
 * Takes in a state and translates/compresses the 2D array into a 1D array with only the information that we care about
 */
void generate_pattern_from_state(struct pattern_cost* patternPtr, struct simplified_state* statePtr){
	//Grab out of the pattern to avoid repeated dereferencing
	int pattern_type = patternPtr->pattern_type;

	int tile;
	//Generation for the first pattern type
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//If we have a pattern tile
			if((tile = statePtr->tiles[i][j]) != 0){	
				//If pattern type is 0, we have a first_half pattern
				if(!pattern_type){
					//We store the index of the tile in the pattern at tile location - 1
					//Example: If tile 1 is at position 8, we store [8] in the 0 spot of the array
					patternPtr->pattern[tile - 1] = i * N + j;
				} else {
					//9 should be at 0 spot
					patternPtr->pattern[tile - (N*N / 2) - 1] = i * N + j;	
				}
			}
		}
	}	
}


/**
 * This function stores the pattern in a temporary linked list in memory. For simplicity,
 * we will first store all patterns in memory, and then write this linked list to a database
 * file after everything has been made
 */
int store_pattern(struct pattern_cost* patternPtr){
	//Define a cursor for linked list traversal
	struct pattern_cost* cursor;	

	//Handle both special cases where we add at the head
	if(!patternPtr->pattern_type){
		//First half pattern
		cursor = patterns_first_half;	
		//Special case, adding at head
		if(patterns_first_half == NULL){
			patterns_first_half = patternPtr;
			//We're done here, don't go any further
			return 0;
		}
	} else {
		//Second half pattern
		cursor = patterns_last_half;
		//special case, adding at head	
		if(patterns_last_half == NULL){
			patterns_last_half = patternPtr;
			//We're done here, don't go any further
			return 0;
		}
	}

	//Iterate through the entire linked list
	while(cursor->next != NULL){
		//If we have the same pattern, we've created a duplicate, which we don't want to store
		if(patterns_same(patternPtr->pattern, cursor->pattern, patternPtr->pattern_length)){
			//It is possible that we've found a lower cost version of this pattern
			if(patternPtr->cost < cursor->cost){
				//We must always store the lowest possible cost, so change it if we find this
				cursor->cost = patternPtr->cost;
			}
			
			//PatternPtr was a repeat, and is useless
			//Free the pointer
			free(patternPtr->pattern);
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
	//Make the next pointer null to alert other iterations
	patternPtr->next = NULL;
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
	//Give the swapped tile value back for heuristic accuracy
	return swapped;
}


/**
 * Move the 0 slider down by 1 row
 */
int move_down(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_row down by 1
	int swapped = swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row+1, statePtr->zero_column, statePtr);	
	//Increment the zero_row to keep the position accurate
	statePtr->zero_row++;
	//Return the tile that was last swapped
	return swapped;
}


/**
 * Move the 0 slider right by 1 column
 */
int move_right(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_column right by 1
	int swapped = swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row, statePtr->zero_column+1, statePtr);	
	//Increment the zero_column to keep the position accurate
	statePtr->zero_column++;
	//Return the tile that was last swapped
	return swapped;
}


/**
 * Move the 0 slider up by 1 row
 */
int move_up(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_row up by 1
	int swapped = swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row-1, statePtr->zero_column, statePtr);	
	//Decrement the zero_row to keep the position accurate
	statePtr->zero_row--;
	//Return the tile that was last swapped
	return swapped;
}


/**
 * Move the 0 slider left by 1 column
 */
int move_left(struct simplified_state* statePtr){
	//Utilize the swap function, move the zero_column left by 1
	int swapped = swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row , statePtr->zero_column-1, statePtr);	
	//Decrement the zero_column to keep the position accurate
	statePtr->zero_column--;
	//Return the tile that was last swapped
	return swapped;
}


/**
 * The thread worker function. This function runs 30 iterations, moving the tile from
 * 10 to max_moves each iteration, and eventually storing the pattern in the appropriate
 * linked list
 */
void* generator_worker(void* thread_params){
	//Grab the max_moves and pattern type from the parameters
	int max_moves = ((struct thread_params*)thread_params)->max_moves;
	int pattern_type = ((struct thread_params*)thread_params)->pattern_type;

	//Set the seed for random generation
	srand(time(NULL));
	int random_move;

	//Save all patterns to a write buffer
	struct pattern_cost* write_buffer[30];

	//Have each thread run 30 iterations
	for(int i = 0; i < 30; i++){
		//Always start at the goal state and work backwards
		struct simplified_state* start = (struct simplified_state*)malloc(sizeof(struct simplified_state));
		//Generate goal state mathematically
		create_goal_state(start, pattern_type);

		//Create the pattern_cost struct
		struct pattern_cost* pc = (struct pattern_cost*)malloc(sizeof(struct pattern_cost));
		//Point the pattern of pc to the newly made start state

		//Determine the pattern_length by type
		if(!pattern_type){
			//The first_half pattern stores the first N*N/2 tiles
			pc->pattern_length = N * N / 2;	
		} else {
			//The last_half pattern store the last N*N/2 tiles, because the 0 tile doesn't count
			pc->pattern_length = (N * N / 2) - 1;
		}
		
		pc->pattern = calloc(sizeof(int), pc->pattern_length);

		//We are at the goal state, so no cost yet
		pc->cost = 0;
		//Set the pattern type appropriately
		pc->pattern_type = pattern_type;

		//Store the value of the tile that was moved
		int moved_tile = 0;

		//Now to generate a pattern randomly, perform i random moves on start
		for(int moves = 10; moves < max_moves; moves++){
			//Get a random move between 0 and 3
			random_move = rand() % 4;

			//Every time we successfully make a random move, we've moved one more away from the goal, so increment cost
			//Move left if possible and random_move is 0
			if(random_move == 0 && start->zero_column > 0){
				//If the last move we did was 1, we'd be undoing all of our work, so we will skip this
				if(start->lastMove == 1){
				//	moves--;
					continue;
				}
			
				//Move the tile left and store what was moved
				moved_tile = move_left(start);
			}

			if(random_move == 1 && start->zero_column < N-1){
				//If the last move we did was 0, we'd be undoing all of our work, so we will skip this
				if(start->lastMove == 0){
				//	moves--;
					continue;
				}
				
				//Move the tile right and store what was moved
				moved_tile = move_right(start);
			} 
			
			if(random_move == 2 && start->zero_row < N-1){
				//If the last move we did was 3, we'd be undoing all of our work, so we will skip this
				if(start->lastMove == 3){
				//	moves--;
					continue;
				}
				
				//Move the tile down and store what was moved
				moved_tile = move_down(start);
			}

			if(random_move == 3 && start->zero_row > 0){
				//If the last move we did was 2, we'd be undoing all of our work, so we will skip this
				if(start->lastMove == 2){
				//	moves--;
					continue;
				}
				
				//Move the tile up and store what was moved
				moved_tile = move_up(start);
			}

			//Set this flag to alert the next iteration
			start->lastMove = random_move;

			//If the tile we moved is not 0, increase the cost
			if(moved_tile != 0){
				pc->cost++;
			}
		}

		//Translate the state into a pattern
		generate_pattern_from_state(pc, start);
		//Save into the write buffer
		write_buffer[i] = pc;
		//Destroy the state as its no longer needed
		destroy_state(start);
	}

	//Store patterns from write buffer
	for(int i = 0; i < 30; i++){
		//Save the pattern in memory if it is not a repeat
	
		if(!pattern_type){
			pthread_mutex_lock(&first_half_lock);
			num_unique_patterns += !store_pattern(write_buffer[i]);
			pthread_mutex_unlock(&first_half_lock);
		} else {
			pthread_mutex_lock(&last_half_lock);
			num_unique_patterns += !store_pattern(write_buffer[i]);
			pthread_mutex_unlock(&last_half_lock);
		}
	}

	return NULL;
}


/**
 * Generate patterns back to a certain traceback_depth
 */
void generate_patterns(int max_moves){
	/**
	 * From before, N puzzles of a depth less than 30 seem easy to solve quickly. So, for our generation, we will
	 * generate states with moves starting from 30 and up to the traceback_depth
	 */

	//Create the thread params structure
	struct thread_params* parameters1 = malloc(sizeof(struct thread_params*));
	struct thread_params* parameters2 = malloc(sizeof(struct thread_params*));

	//Initialize max moves and pattern type for both thread_params
	parameters1->max_moves = max_moves;
	parameters2->max_moves = max_moves;
	parameters1->pattern_type = 0;
	parameters2->pattern_type = 1;

	//For each run, we will use 50 threads
	pthread_t threadArr[50];

	//Each thread will run 20 times
	for(int iter = 0; iter < 100; iter++){
		for(int moves = 5; moves < max_moves; moves++){
			//The first 25 threads will deal with the first half pattern
			for(int i = 0; i < 25; i++){
				pthread_create(&threadArr[i], NULL, generator_worker, parameters1);	
			}

			//Last 25 threads will deal with the last half pattern	
			for(int i = 25; i < 50; i++){	
				pthread_create(&threadArr[i], NULL, generator_worker, parameters2);
			}
	
			//Wait for all threads to finish at this point
			for(int i = 0; i < 50; i++){
				pthread_join(threadArr[i], NULL);
			}
		}

		//Sanity check print for the user
		printf("Still generating, currently generated %d unique patterns\n", num_unique_patterns);
	}
	
	//Be sure to deallocate when done
	free(parameters1);
	free(parameters2);
}


/**
 * Iterate over the given linked list, saving to the database line by line
 */
void save_to_database(FILE* database, struct pattern_cost* pattern_linked_list){
	//Define a cursor to avoid messing with the head reference
	struct pattern_cost* cursor = pattern_linked_list;

	while(cursor != NULL){
		//Very first thing on each line is the pattern type
		fprintf(database, "%d ", pattern_linked_list->pattern_type);
		//Next we print the cost
		fprintf(database, "%d ", cursor->cost);
		//Next is the pattern encoded in the way we've described
		for(int i = 0; i < cursor->pattern_length; i++){
			fprintf(database, "%d ", cursor->pattern[i]);
		}
		//Print a newline for delimiting
		fprintf(database, "\n");
		//Advance the pointer
		cursor = cursor->next;
	}
}


/**
 * The main function simply handles input and makes calls to the appropriate functions
 */
int main(int argc, char** argv){
	//Check to ensure proper number of arguments
	if(argc < 2){
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle\n");
		return 0;
	}

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

	//Filename is always of format "N.patterndb"
	char db_filename[13];

	//Save the filename into a string
	sprintf(db_filename, "%d.patterndb", N);

	//Initialize the two locks that are needed for our thread functions
	pthread_mutex_init(&first_half_lock, NULL);
	pthread_mutex_init(&last_half_lock, NULL);
	
	printf("--------------------------------------------------\n");
	printf("Now generating database for %d puzzle problem\n\n", N);
	
	//Currently optimized for 15 puzzle
	generate_patterns(100);
	
	//Let the user know it worked
	printf("\nSuccess! Generated %d distinct patterns\n", num_unique_patterns);

	//Remove the two locks for our thread functions 
	pthread_mutex_destroy(&first_half_lock);
	pthread_mutex_destroy(&last_half_lock);

	printf("Saving to database file: %s\n\n", db_filename);

	//Open the file for writing
	FILE* database = fopen(db_filename, "w");

	//Save everything into the database
	save_to_database(database, patterns_first_half);
	save_to_database(database, patterns_last_half);

	//Close file when done
	fclose(database);
	printf("------------------ Success! -----------------------\n");

	//All done
	return 0;
}
