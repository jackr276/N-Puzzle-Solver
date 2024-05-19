/**
 * Author: Jack Robbins(jackr276)
 * This program generates pattern databases for the N puzzle
 */

#include <stdio.h>
#include <stdlib.h>


//Define a global variable for the value of N
int N;


int main(int argc, char** argv){
	//Check to ensure proper number of arguments
	if(argc < 2){
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle");
	}

	//Get the N value from the user to generate the DB
	if(sscanf(argv[1], "%d", &N) == 1){	
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./generate_pattern_db <N>\n Where <N> is the row/column number of the N puzzle");

	}





	return 0;
}
