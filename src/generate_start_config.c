/**
 * Author: Jack Robbins
 * This simple program generates a 15 puzzle problem configuration that has been 
 * "messed up" according to the inputted specification
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//For random move generation
#include <time.h>

//The gem puzzle is 4 rows * 4 columns = 16 tiles in total
#define N 4


//Define a simplified state struct for the generation process. We only need the tiles and zero_row and column
struct simplified_state{
	//Define the tiles as an N by N array
	int tiles[N][N];
	short zero_row, zero_column;
};


void swap(int row1,int column1,int row2,int column2, struct simplified_state * pstate){
	int tile = pstate->tiles[row1][column1];
	pstate->tiles[row1][column1]=pstate->tiles[row2][column2];
	pstate->tiles[row2][column2]=tile;
}


/* 0 goes down by a row */
void move_down(struct simplified_state * pstate){
	swap(pstate->zero_row, pstate->zero_column, pstate->zero_row+1, pstate->zero_column, pstate); 
	pstate->zero_row++;
}

/* 0 goes right by a column */
void move_right(struct simplified_state * pstate){
	swap(pstate->zero_row, pstate->zero_column, pstate->zero_row, pstate->zero_column+1, pstate); 
	pstate->zero_column++;
}

/* 0 goes up by a row */
void move_up(struct simplified_state * pstate){
	swap(pstate->zero_row, pstate->zero_column, pstate->zero_row-1, pstate->zero_column, pstate); 
	pstate->zero_row--;
}

/* 0 goes left by a column */
void move_left(struct simplified_state* pstate){
	swap(pstate->zero_row, pstate->zero_column, pstate->zero_row, pstate->zero_column-1, pstate); 
	pstate->zero_column--;
}

void print_a_state(struct simplified_state *pstate, int format) {
	int i,j;
	for (i=0;i<N;i++) {
		for (j=0;j<N;j++) printf("%2d ",pstate->tiles[i][j]);
		if(format==0)
			printf("\n");
	}
	printf("\n");
}

int main(int argc,char** argv) {
	//If there aren't enough program arguments, print an error and exit
	if(argc != 2){
		printf("\nIncorrect number of program arguments.\n");
		printf("Usage: ./generate_start_config <n>\nWhere <n> is initial complexity.\n\n");
		exit(1);
	}

	//Once we know there are the right amount of arguments, attempt to scan the number of moves into numMoves
	int num_moves;
	//If scanning into a decimnal doesn't work, we know something went wrong
	if(sscanf(argv[1], "%d", &num_moves) != 1){
		//Print an error and exit
		printf("\nIncorrect type of program arguments.\n");
		printf("Usage: ./generate_start_config <n>\nWhere <n> is a positive integer.\n\n");
		exit(1);
	}

	int i,j,k;
	//Create the 
	struct simplified_state* statePtr = (struct simplified_state*)malloc(sizeof(struct simplified_state));

	for(int index=1; index<N*N; index++){
		j=(index-1)/N;
		k=(index-1)%N;
		statePtr->tiles[j][k]=index;

	}
	statePtr->tiles[N-1][N-1]=0;
	statePtr->zero_row = N-1;
	statePtr->zero_column = N-1;

	srand(time(NULL));
	i=0;
	do{
		j = rand();
		k = j % 4;
  		if( k == 0 && statePtr->zero_row>0){
     			move_up(statePtr);
			i++;
		}
  		if( k == 1 && statePtr->zero_row<N-1){
     			move_down(statePtr);
			i++;
		}
  		if( k == 2 && statePtr->zero_column>0){
     			move_left(statePtr);
			i++;
		}
  		if( k == 3 && statePtr->zero_column<N-1){
     			move_right(statePtr);
			i++;
		}
  	}while(i<num_moves);

	printf("Initial state in puzzle format:\n");
	print_a_state(statePtr, 0);

	printf("Initial state in one line, for use in solve.c:\n");
	print_a_state(statePtr, 1);

	return 0;
}
