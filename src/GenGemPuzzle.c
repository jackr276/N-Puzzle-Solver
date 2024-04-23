/**
 * Author: Jack Robbins
 * This simple program generates a 15 puzzle problem configuration that has been 
 * "messed up" according to the inputted specification
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Define some useful constants to avoid magic numbers
//The gem puzzle is 4 rows * 4 columns = 16 tiles in total
#define N 4
#define NxN (N*N)


struct simplified_state {
	int tiles[N][N];
	short zero_row, zero_column;   /* location (row and colum) of blank tile 0 */
};

int goal_rows[NxN];
int goal_columns[NxN];

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

int main(int argc,char **argv) {
	int i,j,k,index,count;

	if(argc!=2 || sscanf(argv[1], "%d", &count)!=1) {
		printf("%s #steps\n", argv[0]);
		exit(1);
	}

	struct simplified_state* pstate=(struct simplified_state*) malloc(sizeof(struct simplified_state));
	goal_rows[0]=3;
	goal_columns[0]=3;

	for(index=1; index<NxN; index++){
		j=(index-1)/N;
		k=(index-1)%N;
		goal_rows[index]=j;
		goal_columns[index]=k;
		pstate->tiles[j][k]=index;

	}
	pstate->tiles[N-1][N-1]=0;	      /* empty tile=0 */
	pstate->zero_row = N-1;
	pstate->zero_column = N-1;

	srand(time(NULL));
	i=0;
	do{
		j = rand();
		k = j % 4;
  		if( k == 0 && pstate->zero_row>0){
     			move_up(pstate);
			i++;
		}
  		if( k == 1 && pstate->zero_row<N-1){
     			move_down(pstate);
			i++;
		}
  		if( k == 2 && pstate->zero_column>0){
     			move_left(pstate);
			i++;
		}
  		if( k == 3 && pstate->zero_column<N-1){
     			move_right(pstate);
			i++;
		}
  	}while(i<count);

	printf("Initial state in puzzle format:\n");
	print_a_state(pstate, 0);

	printf("Initial state in one line, for use in solve.c:\n");
	print_a_state(pstate, 1);

	return 0;
}
