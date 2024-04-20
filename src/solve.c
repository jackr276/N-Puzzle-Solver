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


/**
 * The initialization function takes in the command line arguments and translates them into the initial
 * state. 
 * Note: Assumes a correct number of command line arguments(16 numbers), must be checked by caller
 */
void initialize(char **argv){
	int j,k,index, tile;

	/* Begin by creating the start state */
	start=(struct state*)malloc(sizeof(struct state));
	//Start at 1, argv[0] is program name
	index = 1;
	//Insert everything into the tiles matrix
	for (j=0;j<N;j++){
		for (k=0;k<N;k++){
			//Grab the specific tile number from the arguments and place it into the start state	
			tile=atoi(argv[index++]);
			start->tiles[j][k]=tile;

			//If we found the zero tile, update the zero row and column
			if(tile==0){
				start->zero_row=j;
				start->zero_column=k;
			}
		}
	}

	/* Initialize everything else in the start state */
	start->total_cost=0;
	start->current_travel=0;
	start->heuristic_cost=0;
	start->next=NULL;
	//Important -- must have no predecessor(root of search tree)
	start->predecessor=NULL;

	//Print to the console for the user
	printf("Initial state\n");
	print_a_state(start);

	//Initialize the goal state(all tiles in order, 0 at the very last one)
	goal=(struct state*)malloc(sizeof(struct state));
	goal_rows[0]=3;
	goal_columns[0]=3;

	for(index=1; index < NxN; index++){
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


/**
 * States will be merged into the fringe linked list in a priority queue fashion by their total_cost values. The lower the value,
 * the higher the priority. This function merges the state pointed to by i into the fringe.
 * Note: This function assumes that succ_states[i] is NOT NULL, must be checked by caller
 */
void priority_queue_insert(int i){
	//Initialize the cursor to be the start of the fringe
	struct state* cursor = fringe;

	//Grab the priority for convenience
	int priority = succ_states[i]->total_cost;

	//Special case: inserting at the head
	if(cursor == NULL || priority < cursor->total_cost){
		//Set the succ_states[i] to point to the old head(fringe)
		succ_states[i]->next = fringe;
		//Set fringe to the succ_states[i]
		fringe = succ_states[i];
		//Exit once done
		return;
	}

	//Iterate over the list as long as the next cursor's total cost is not more than priority
	while(cursor->next != NULL && cursor->next->total_cost < priority){
		cursor = cursor->next;	
	}

	//Once we get here, the cursor should be on the node right before the node to be inserted
	//Perform the insertion, check for special cases(like the cursor being the tail)
	if(cursor->next == NULL){
		//If this is the case, we have the tail, so just append the succ_states[i]
		cursor->next = succ_states[i];
	} else {
		//Otherwise, we have to break the linked list and insert succ_states[i] into the right spot
		//Save the next state
		struct state* temp = cursor->next;
		//Insert succ_states[i] after cursor
		cursor->next = succ_states[i];
		//Reattach the rest of the linked list
		succ_states[i]->next = temp;
	}
	
}


/**
 * This function simply iterates through succ_states, passing the appropriate indices along to priority_queue_insert if the pointers
 * are not null
 */
void merge_to_fringe() { 
	//Iterate through succ_states, if the given state is not null, call the priority_queue_insert function on it
	for(int i = 0; i < 4; i++){
		if(succ_states[i] != NULL){
			priority_queue_insert(i);
		}
	}	
}


/**
 * Update the prediction function for the state pointed to by succ_states[i]. If this pointer is null, simply skip updating
 * and return
 */ 
void update_prediction_function(int i) {
	struct state* statePtr = succ_states[i];
	//If statePtr is null, this state was a repeat and has been freed, so don't calculate anything
	if(statePtr == NULL){
		return;
	}

	//The current_travel of the state has already been updated by stateCopy, so we only need to find the heuristic_cost
	statePtr->heuristic_cost = 0;

	//For heuristic_cost, we will use the manhattan distance from each tile to where it should be. Conveniently, each tile 1-15 should
	//be in position 0-14, so we can find manhattan distance be doing the sum of the absolute difference in coordinates from a number's current
	//state position to its goal state position
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			int selected_num, goal_rowCor, goal_colCor;	
			//grab the number to be examined
			selected_num = statePtr->tiles[i][j];
	
			//0 is a special case, should be in the very last cell
			if(selected_num == 0){
				goal_rowCor = goal_colCor = N-1;
			//Otherwise mathematically find the needed position
			} else {
				//Goal row coordinate is the index of the number divided by number of rows
				goal_rowCor = (selected_num - 1) / N;
				//Goal column coordinate is the index modulated by column length 
				goal_colCor = (selected_num - 1) % N;
			}
		
			//Manhattan distance is the absolute value of the x distance and the y distance
			statePtr->heuristic_cost += abs(i - goal_rowCor) + abs(j - goal_colCor);
		}
	}
	//Once we have the heuristic_cost, update the total_cost
	statePtr->total_cost = statePtr->heuristic_cost + statePtr->current_travel;
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


/**
 * Check to see if the state at position i in the given linkedList is repeating. If it is, free it and set the pointer to be null
 */
void check_repeating(int i, struct state* stateLinkedList){ 	
	//If succ_states[i] is NULL, no need to check anything
	if(succ_states[i] == NULL){
		return;
	}

	//Get a cursor to iterate over the linkedList	
	struct state* cursor = stateLinkedList;
	//Go through the linkedList, if we ever find an element that's the same, break out and free the pointer
	while(cursor != NULL){
		//If the states match, we free the pointer and exit the loop
		if(states_same(succ_states[i], cursor)){
			//Free the duplicate state
			free(succ_states[i]);
			//Set the pointer to be null as a warning
			succ_states[i] = NULL;
			break;
		}
		//Move to the next state in the linkedList
		cursor = cursor->next;
	}
	//Once we get here, the state at i either survived and isn't NULL, or was freed and set to NULL
}


int main(int argc,char **argv) {
	//Check if the number of arguments is correct. If not, exit the program and print an error
	if(argc != 17){
		//Give an error message
		printf("Incorrect number of program arguments. Please retry with a correct configuration.\n");
		return 1;
	}

	int iter;
	struct state *curr_state, *solution_path;
	int i, pathlen=0;

	solution_path=NULL;
	//Initialize the goal and start states 
	initialize(argv);
	//Put the start state into the fringe to begin the search
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
			/* print out the states on the list */
			while(solution_path != NULL){
				print_a_state(solution_path);
				solution_path=solution_path->next;
			}	

			//We've found a solution, so the program should exit
			return 0;	
		}

		//Generate successors to the current state once we know it isn't a solution
		generate_successors(curr_state);

		for(i=0;i<4;i++){
			//Check each successor state against fringe and closed to see if it is repeating
			check_repeating(i,fringe); 
			check_repeating(i,closed);
			//Update the prediction function on states that don't repeat
			update_prediction_function(i); 
		}
	
		//Add all necessary states to fringe now that we have checked for repeats and updated predictions 
		merge_to_fringe(); 
		
		//Move the current state into closed
		curr_state->next=closed;
		//Maintain closed properly
		closed=curr_state;


		if(iter++ %1000 == 0) printf("iter %d\n", iter);
	}
	
	//If we end up here, fringe became NULL with no goal configuration found, so there is no solution
	printf("No solution.");
	return 0;
}