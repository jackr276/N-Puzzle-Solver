/**
 * Jack Robbins
 * 04/20/2024
 * CS-288, Homework 07, Only problem(A* solver)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Grid is 4 by 4
#define N 4
//Following defined for convenience
#define NxN (N*N)
#define TRUE 1
#define FALSE 0


/**
 * Defines a type of state, which is a structure, that represents a configuration in the gem puzzle game
 */
struct state{
	//There are 4x4 = 16 tiles in the game
	int tiles[N][N];
	//For A*, define the total_cost, how far the tile has traveled, and heuristic cost
	int total_cost, current_travel, heuristic_cost;
	//location (row and colum) of blank tile 0
	short zero_row, zero_column;
	//The next state in the linked list(fringe or closed), NOT a successor
	struct state* next;
	//The predecessor of the current state, used for tracing back a solution	
	struct state* predecessor;			
};


/* The following global variables are defined for convenience */

int goal_rows[NxN];
int goal_columns[NxN];
struct state* start;
struct state* goal;
struct state* fringe = NULL;
struct state* closed = NULL;
struct state* succ_states[4];


/**
 * Prints out a state by printing out the positions in the 4x4 grid
 */
void print_a_state(struct state* statePtr) {
	//Go through tile by tile and print out
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++){ 
			printf("%2d ", statePtr->tiles[i][j]);
		}
		//Print a newline to represent the row change
		printf("\n");
	}
	printf("\n");
}


void initialize(char **argv){
	int i,j,k,index, tile;

	start=(struct state*)malloc(sizeof(struct state));
	index = 1;
	for (j=0;j<N;j++)
		for (k=0;k<N;k++) {
			tile=atoi(argv[index++]);
			start->tiles[j][k]=tile;
			if(tile==0) {
				start->zero_row=j;
				start->zero_column=k;
			}
		}
	start->total_cost=0;
	start->current_travel=0;
	start->heuristic_cost=0;
	start->next=NULL;
	start->predecessor=NULL;
	printf("initial state\n");
	print_a_state(start);

	//Initialize the goal state(all tiles in order, 0 at the very last one)
	goal=(struct state*)malloc(sizeof(struct state));
	goal_rows[0]=3;
	goal_columns[0]=3;

	for(index=1; index<NxN; index++){
		j=(index-1)/N;
		k=(index-1)%N;
		goal_rows[index]=j;
		goal_columns[index]=k;
		goal->tiles[j][k]=index;
	}
	//Position of empty tile
	goal->tiles[N-1][N-1]=0;
	goal->total_cost=0;
	goal->current_travel=0;
	goal->heuristic_cost=0;
	goal->next=NULL;
	printf("goal state\n");
	print_a_state(goal);

}

/* merge the remaining states pointed by succ_states[] into fringe. 
 * Insert states based on their f values --- keep f values sorted. */
void merge_to_fringe() { 
}

/*update the f,g,h function values for a state pointed by a pointer in succ_states */
void update_fgh(int i) {
	struct state* pstate = succ_states[i];

		
}


/**
 * A simple function that swaps two tiles in the provided state
 * Note: The swap function assumes all row positions are valid, this must be checked by the caller
 */
void swap(int row1, int column1, int row2, int column2, struct state* statePtr){
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
void move_down(struct state* statePtr){
	//Utilize the swap function, move the zero_row down by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row+1, statePtr->zero_column, statePtr);	
	//Increment the zero_row to keep the position accurate
	statePtr->zero_row++;
}


/**
 * Move the 0 slider right by 1 column
 */
void move_right(struct state* statePtr){
	//Utilize the swap function, move the zero_column right by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row, statePtr->zero_column+1, statePtr);	
	//Increment the zero_column to keep the position accurate
	statePtr->zero_column++;
}


/**
 * Move the 0 slider up by 1 row
 */
void move_up(struct state* statePtr){
	//Utilize the swap function, move the zero_row up by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row-1, statePtr->zero_column, statePtr);	
	//Decrement the zero_row to keep the position accurate
	statePtr->zero_row--;
}


/**
 * Move the 0 slider left by 1 column
 */
void move_left(struct state* statePtr){
	//Utilize the swap function, move the zero_column left by 1
	swap(statePtr->zero_row, statePtr->zero_column, statePtr->zero_row , statePtr->zero_column-1, statePtr);	
	//Decrement the zero_column to keep the position accurate
	statePtr->zero_column--;
}


/**
 * Performs a "deep copy" from the predecessor to the successor
 */
void copyState(struct state* predecessor, struct state* successor){
	//Copy over the tiles array
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
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
 * This function generates all possible successors to a state and stores them in the successor array
 * Note: 4 successors are not always possible, if a successor isn't possible, NULL will be put in its place 
 */
void generate_successors(struct state* predecessor){
	//Create four pointers, one for each possible move, and initialize to NULL by default
	struct state* leftMove = NULL;
	struct state* rightMove = NULL;
	struct state* upMove = NULL;
	struct state* downMove = NULL;
	
	//Generate successor by moving left one if possible
	if(predecessor->zero_column > 0){
		//Create a new state
		leftMove = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy on the state
		copyState(predecessor, leftMove);
		//Move right by one
		move_left(leftMove);
	}
	//Put leftMove into the array
	succ_states[0] = leftMove;

	//Generate successor by moving right one if possible
	if(predecessor->zero_column < N-1){
		//Create a new state
		rightMove = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy on the state
		copyState(predecessor, rightMove);
		//Move right by one
		move_right(rightMove);
	}
	//Put rightMove into the array
	succ_states[1] = rightMove;
	
	//Generate successor by moving down one if possible
	if(predecessor->zero_row < N-1){
		//Create a new state
		downMove = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy on the state
		copyState(predecessor, downMove);
		//Move down by one
		move_down(downMove);
	}
	//Put downMove into the array
	succ_states[2] = downMove;
	
	//Generate successor by moving down one if possible
	if(predecessor->zero_row > 0){
		//Create a new state
		upMove = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy on the state
		copyState(predecessor, upMove);
		//Move up by one
		move_up(upMove);
	}
	//Put upMove into the array
	succ_states[3] = upMove;
}





/**
 * A simple helper function that will tell if two states are the same. To be used for filtering
 */
int states_same(struct state* a, struct state* b) {
	//By default, they are not the same
	int same=FALSE;

	//Utilize memcmp function on the tiles for convenience
	if (memcmp(a->tiles, b->tiles, sizeof(int)*NxN) == 0){
		same=TRUE;
	}
	
	return same;
}


/* Filtering:
 * check the state pointed by succ_states[i] to determine whether this state is repeating.
 * free the state if it is repeating. 
 */ 
void filter(int i, struct state *pstate_list){ 
	struct state *pstate = succ_states[i];
	...
}


int main(int argc,char **argv) {
	int iter,cnt;
	struct state *curr_state, *cp, *solution_path;
	int ret, i, pathlen=0, index[N-1];

	solution_path=NULL;
	initialize(argv);	/* init initial and goal states */
	fringe=start; 

	iter=0; 
	while (fringe!=NULL) {	
		curr_state=fringe;
		fringe=fringe->next;  /* get the first state from fringe to expand */
		

		if(states_same(curr_state,goal)){ /* a solution is found */
			do{ /* trace back and add the states on the path to a list */
				curr_state->next=solution_path;
				solution_path=curr_state;
				curr_state=curr_state->predecessor;
				pathlen++;
			} while(curr_state!=NULL);
			printf("Path (lengh=%d):\n", pathlen); 
			curr_state=solution_path;
			... /* print out the states on the list */
			break;
		}
		expand(curr_state);       /* generate new states */


		for(i=0;i<4;i++){
			filter(i,fringe); /* Check succ_states[i] agaist fringe to see if it is repeatng */
			filter(i,closed); /* Check succ_states[i] agaist closed to see if it is repeatng */
			update_fgh(i); /* evaluate the state if it is not repeating */
		}


		merge_to_fringe(); /* add new states to fringe */

		curr_state->next=closed;
		closed=curr_state;	/* curr_state has been checked/expand, add it to closed */
		/* print out something so that you know your program is still making progress 
		 */
		if(iter++ %1000 == 0) printf("iter %d\n", iter);
	}
	return 0;
} /* end of main */
