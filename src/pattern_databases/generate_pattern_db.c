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


void generate_patterns(FILE* db, struct simplified_state* statePtr, int group_size){
	
}


int contained_in_db(FILE* db, struct simplified_state* statePtr){


	return 0;
}

void example(char* input){
	printf("Input was: %s\n", input);
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

	printf("%s\n", db_filename);

	FILE* db = fopen(db_filename, "a+");	
	fprintf(db, "I work");

	fclose(db);

	return 0;
}
