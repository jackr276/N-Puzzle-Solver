/**
 * Author: Jack Robbins
 * This program generates pattern databases for the N puzzle
 */

#include <stdio.h>
#include <stdlib.h>


//Define a global variable for the value of N
int N;

//Define a simplified state struct for the generation process. We only need the tiles and zero_row and column
struct simplified_state{
	//Define the tiles as an N by N array
	int** tiles;
	short zero_row, zero_column;
};


struct pattern_cost{
	struct simplified_state* pattern;
	int cost;
	struct pattern_cost* next;
};


//We will start at the goal and generate backwards
struct simplified_state* goal_state;

//We will keep a linked list of all patterns seen to avoid storing repeats
struct pattern_cost* patterns;


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


	for(int i = 0; i<N; i++){
		for(int j = 0; j<N; j++){
			printf("%2d ", initial->tiles[i][j]);
		}
		printf("\n");
	}
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


/********************************************************************************************
 * Basic Idea: Store some pattern and how many moves it takes to go from that pattern to goal
 * Encoding Format -> attempt to minimize space usage
 * Idea: there are 16 positions on the board, numbers are 0-15
 * --- NUMBER POSITION -----
 *   	ROW_POSITION . . . .   MOVES_FROM_GOAL 
 * With this, we can identify the 
 *
 * So for Example:
 * 1  2  3  0
 * 5  6  7  4
 * 9  10 11 8
 * 13 14 15 12
 * Can be encoded as
 * 1 2 3 0 5 6 7 4 9 10 11 8 13 14 15 12 3, where 3 is how many moves from goal this state is 

 *******************************************************************************************/

int contained_in_db(struct simplified_state* statePtr){
	

	return 0;
}



void generate_patterns(FILE* db, struct simplified_state* statePtr, int traceback_depth){
		
}


int main(int argc, char** argv){
	//Check to ensure proper number of arguments
	if(argc < 3){
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N> <G>\n Where <N> is the row/column number of the N puzzle, <G> is group size\n");
	}

	//Which group size do we want to generate patterns for
	int group_size;

	//Get the N and G value from the user for which database that they want to generate 
	if(sscanf(argv[1], "%d", &N) != 1 || sscanf(argv[2], "%d", &group_size) != 1){	
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle, <G> is group size\n");
	}

	//Filename is always of format "N_G.patterndb"
	char db_filename[14];

	//Save the filename into a string
	sprintf(db_filename, "%d_%d.patterndb", N, group_size);

	printf("Generating database: %s\n\n", db_filename);

	FILE* db = fopen(db_filename, "w");	

	//Be sure to close the file
	fclose(db);
	goal_state = (struct simplified_state*)malloc(sizeof(struct simplified_state));
	create_goal_state(goal_state);
	destroy_state(goal_state);

	return 0;
}
