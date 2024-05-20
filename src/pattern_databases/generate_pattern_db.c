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

//We will start at the goal and generate backwards
struct simplified_state* goal_state;

void create_goal_state(struct simplified_state* initial){
	initial->tiles = malloc(sizeof(int*)*N);

	for(int i = 0; i < N; i++){
		initial->tiles[i] = malloc(sizeof(int) * N);
	}

	int row, col;
	for(int i = 0; i < N * N; i++){
		row = i / N;
		col = i % N;
		initial->tiles[row][col] = i+1;
	}

	initial->tiles[N-1][N-1] = 0;

	for(int i = 0; i<N; i++){
		for(int j = 0; j<N; j++){
			printf("%2d ", initial->tiles[i][j]);
		}
		printf("\n");
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
 * 1  2  3  0
 * 5  6  7  4
 * 9  10 11 8
 * 13 14 15 12
 * Can be encoded as
 * 1 2 3 0 5 6 7 4 9 10 11 8 13 14 15 12 3, where 3 is how many moves from goal this state is 

 *******************************************************************************************/

void generate_patterns(FILE* db, struct simplified_state* statePtr, int group_size){

}


int contained_in_db(FILE* db, struct simplified_state* statePtr){


	return 0;
}


int main(int argc, char** argv){
	//Check to ensure proper number of arguments
	if(argc < 3){
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N> <G>\n Where <N> is the row/column number of the N puzzle, <G> is group size\n");
	}

	//Which group size do we want to generate patterns for
	int group_size;

	//Get the N value from the user for which database that they want to generate 
	if(sscanf(argv[1], "%d", &N) != 1 || sscanf(argv[2], "%d", &group_size) != 1){	
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle, <G> is group size\n");
	}

	//Filename is always of format "N_G.patterndb"
	char db_filename[14];

	sprintf(db_filename, "%d_%d.patterndb", N, group_size);

	printf("Generating database: %s\n", db_filename);

	FILE* db = fopen(db_filename, "a+");	

	fclose(db);
	goal_state = (struct simplified_state*)malloc(sizeof(struct simplified_state));
	create_goal_state(goal_state);

	return 0;
}
