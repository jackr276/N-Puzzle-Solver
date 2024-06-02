/**
 * Author: Jack Robbins
 * This c file contains the implementations for functions in puzzle.h
 */


#include "puzzle.h"


/*================================= Global variables for convenience =========================== */
//The fringe is the set of all states open for exploration. It is maintained as a linked list
struct state* fringe = NULL;
//Closed is an array containing all sets previously examined. This is used to avoid repeating
struct state** closed;
//Define an initial starting size of 5000
int closed_max_size = 5000;
//We will keep a reference to the next available closed index
int next_closed_index = 0;
struct state* ex;
/*============================================================================================== */


/**
 * The initialize_state function takes in a pointer to a state and reserves the appropriate space for the dynamic array
 * that holds the tiles 
 */
void initialize_state(struct state** statePtr, const int N){
	//Declare all of the pointers needed for each row
	(*statePtr)->tiles = (short**)malloc(sizeof(short*) * N);

	//For each row, allocate space for N integers
	for(int i = 0; i < N; i++){
		(*statePtr)->tiles[i] = (short*)malloc(sizeof(short) * N);
	}
}


/**
 * The destroy_state function does the exact reverse of the initialize_state function to properly free memory
 */
void destroy_state(struct state* statePtr, const int N){
	//Go through row by row, freeing each one
	for(int i = 0; i < N; i++){
		free(statePtr->tiles[i]);
	}

	//Once all rows are free, free the array of row pointers
	free(statePtr->tiles);
}


/**
 * Prints out a state by printing out the positions in the 4x4 grid
 */
void print_state(struct state* statePtr, const int N){
	//Go through tile by tile and print out
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++){
			//Support printing of states with 2 or 3 digit max integers
			if(N < 11){	
				//With numbers less than 11, N^2 is at most 99, so only 2 digits needed
				printf("%2d ", statePtr->tiles[i][j]);
			} else {
				//Ensures printing of large states will not be botched
				printf("%3d ", statePtr->tiles[i][j]);
			}
		}
		//Print a newline to represent the row change
		printf("\n");
	}
	printf("\n");
}


/**
 * Performs a "deep copy" from the predecessor to the successor
 */
void copy_state(struct state** predecessor, struct state** successor, const int N){
	//Copy over the tiles array
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//Copy tile by tile
			(*successor)->tiles[i][j] = (*predecessor)->tiles[i][j];
		}
	}

	//Initialize the current travel to the predecessor travel + 1
	(*successor)->current_travel = (*predecessor)->current_travel+1;
	//Copy the zero row and column position
	(*successor)->zero_row = (*predecessor)->zero_row;
	(*successor)->zero_column = (*predecessor)->zero_column;
	//Initialize the successor's next to be null
	(*successor)->next = NULL;
	//Set the successors predecessor
	(*successor)->predecessor = *predecessor;
}


/**
 * A simple function that swaps two tiles in the provided state
 * Note: The swap function assumes all row positions are valid, this must be checked by the caller
 */
static void swap(int row1, int column1, int row2, int column2, struct state** statePtr){
	//Store the first tile in a temp variable
	short tile = (*statePtr)->tiles[row1][column1];
	//Put the tile from row2, column2 into row1, column1
	(*statePtr)->tiles[row1][column1] = (*statePtr)->tiles[row2][column2];
	//Put the temp in row2, column2
	(*statePtr)->tiles[row2][column2] = tile;
}



/**
 * Move the 0 slider down by 1 row
 */
void move_down(struct state** statePtr){
	//Utilize the swap function, move the zero_row down by 1
	swap((*statePtr)->zero_row, (*statePtr)->zero_column, (*statePtr)->zero_row+1, (*statePtr)->zero_column, statePtr);	
	//Increment the zero_row to keep the position accurate
	(*statePtr)->zero_row++;
}


/**
 * Move the 0 slider right by 1 column
 */
void move_right(struct state** statePtr){
	//Utilize the swap function, move the zero_column right by 1
	swap((*statePtr)->zero_row, (*statePtr)->zero_column, (*statePtr)->zero_row, (*statePtr)->zero_column+1, statePtr);	
	//Increment the zero_column to keep the position accurate
	(*statePtr)->zero_column++;
}


/**
 * Move the 0 slider up by 1 row
 */
void move_up(struct state** statePtr){
	//Utilize the swap function, move the zero_row up by 1
	swap((*statePtr)->zero_row, (*statePtr)->zero_column, (*statePtr)->zero_row-1, (*statePtr)->zero_column, statePtr);	
	//Decrement the zero_row to keep the position accurate
	(*statePtr)->zero_row--;
}


/**
 * Move the 0 slider left by 1 column
 */
void move_left(struct state** statePtr){
	//Utilize the swap function, move the zero_column left by 1
	swap((*statePtr)->zero_row, (*statePtr)->zero_column, (*statePtr)->zero_row , (*statePtr)->zero_column-1, statePtr);	
	//Decrement the zero_column to keep the position accurate
	(*statePtr)->zero_column--;
}


/**
 * A simple helper function that will tell if two states are the same. To be used for filtering
 */
int states_same(struct state* a, struct state* b, const int N){
	//Efficiency speedup -- if zero row and column aren't equal, return false
	if(a->zero_row != b->zero_row || a->zero_column != b->zero_column){
		return 0;
	}

	//Go through each row in the dynamic tile matrix in both states
	for(int i = 0; i < N; i++){
		//We can use memcmp to efficiently compare the space pointed to by each pointer
		if(memcmp(a->tiles[i], b->tiles[i], sizeof(short) * N) != 0){
			//If we find a difference, return 0
			return 0;
		}
	}

	//Return 1 if same	
	return 1;
}


/**
 * Update the prediction function for the state pointed to by succ_states[i]. If this pointer is null, simply skip updating
 * and return. This is a generic algorithm, so it will work for any size N
 */ 
void update_prediction_function(struct state** statePtr, const int N){
	//If statePtr is null, this state was a repeat and has been freed, so don't calculate anything
	if(*statePtr == NULL){
		return;
	}

	//The current_travel of the state has already been updated by stateCopy, so we only need to find the heuristic_cost
	(*statePtr)->heuristic_cost = 0;

	/**
	* For heuristic_cost, we will use the manhattan distance from each tile to where it should be.
	* Conveniently, each tile 1-15 should be in position 0-14, so we can find manhattan distance by
	* doing the sum of the absolute difference in coordinates from a number's current state position
	* to its goal state position
	*/

	//Declare all needed variables
	short selected_num, goal_rowCor, goal_colCor;
	//Keep track of the manhattan distance
	int manhattan_distance;
	
	//Go through each tile in the state and calculate the heuristic_cost
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//grab the number to be examined
			selected_num = (*statePtr)->tiles[i][j];

			//We do not care about 0 as it can move, so skip it
			if(selected_num == 0){
				continue;
			}

			//Otherwise mathematically find the needed position
			//Goal row coordinate is the index of the number divided by number of rows
			goal_rowCor = (selected_num - 1) / N;
			//Goal column coordinate is the index modulated by column length 
			goal_colCor = (selected_num - 1) % N;
				

			//Manhattan distance is the absolute value of the x distance and the y distance
			manhattan_distance = abs(i - goal_rowCor) + abs(j - goal_colCor);	
		
			//Add manhattan distance for each tile
			(*statePtr)->heuristic_cost += manhattan_distance;
		}
	}
		
	/**
	 * Now we must calculate the linear conflict heuristic. This heuristic takes two tiles in their goal row
	 * or goal column and accounts for the fact that for each tile to be moved around, it actually takes
	 * at least 2 additional moves. Given two tiles in their goal row, 2 additional vertical moves are added
	 * to manhattan distance for each row/column amount that they have to move
	 * 
	 */

	//We initially have no linear conflicts
	int linear_conflicts = 0;
	//Declare for convenience
	short left, right, above, below;
	//Also declare goal row coordinates for convenience
	short goal_rowCor_left, goal_rowCor_right, goal_colCor_above, goal_colCor_below;

	//Check each row for linear conflicts
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N-1; j++){
			//Grab the leftmost tile that we'll be comparing to
			left = (*statePtr)->tiles[i][j];

			//If this tile is 0, it's irrelevant so do not explore further
			if(left == 0){
				continue;
			}
			
			//Now go through every tile in the row after left, this is what makes this generalized linear conflict
			for(int k = j+1; k < N; k++){
				//Grab right tile for convenience
				right = (*statePtr)->tiles[i][k];

				//Again, if the tile is 0, no use in wasting cycles with it
				if(right == 0){
					continue;
				}

				//Check if both tiles are in their goal row
				goal_rowCor_left = (left - 1) / N;
				goal_rowCor_right = (right - 1) / N;

				//If these tiles are not BOTH their goal row, linear conflict does not apply
				//This is conterintuitive, but as it turns out makes an enormous difference
				if(goal_rowCor_left != goal_rowCor_right || goal_rowCor_right != i){
					continue;
				}
			
				//If the tiles are swapped, we have a linear conflict
				if(left > right){
					//To be more informed, we should add 2 moves for EACH time we have to swap
					linear_conflicts++;
				}
			}	
		}
	}

	//Now check each column for linear conflicts 
	for(int i = 0; i < N-1; i++){
		for(int j = 0; j < N; j++){
			//Grab the abovemost tile that we'll be comparing to
			above = (*statePtr)->tiles[i][j];

			//If this tile is 0, it's irrelevant so do not explore further
			if(above == 0){
				continue;
			}

			//Now go through every tile in the column below "above", this is what makes it generalized linear conflict
			for(int k = i+1; k < N; k++){
				//Grab the below tile for convenience
				below = (*statePtr)->tiles[k][j];

				//We don't care about the 0 tile, skip if we find it
				if(below == 0){
					continue;
				}

				//Check if both tiles are in their goal column
				goal_colCor_above = (above - 1) % N;
				goal_colCor_below = (below - 1) % N;

				//If these tiles are not BOTH in their goal column, linear conflict does not apply
				//This is counterintutive, but as it turns out makes an enormous difference
				if(goal_colCor_below != goal_colCor_above || goal_colCor_above != j){
					continue;
				}

				//If above is more than below, we have a linear conflict
				if(above > below){
					//To be more informed, we should add 2 moves for EACH time we have to swap
					linear_conflicts++;				
				}
			}
		}
	}

	//Once we have calculated the number of linear conflicts, we add it into the heuristic cost
	//For each linear conflict, a minimum of 2 additional moves are required to swap tiles, so add 2 to the heuristic_cost
	(*statePtr)->heuristic_cost += linear_conflicts * 2;

	//Once we have the heuristic_cost, update the total_cost
	(*statePtr)->total_cost = (*statePtr)->heuristic_cost + (*statePtr)->current_travel;
}



/**
 * The initialization function takes in the command line arguments and translates them into the initial
 * state. It also initializes the goal state mathematically, as it is always the same
 * Note: Assumes a correct number of command line arguments(16 numbers), must be checked by caller
 */
void initialize_start_goal(char** argv, struct state** start_state, struct state** goal_state, const int N){
	/* Begin by creating the start state */

	//Create the start state itself
	*start_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate memory needed in the start_state
	initialize_state(start_state, N);

	//Start at 1, argv[0] is program name and argv has been adjusted up by 1 to only contain the start state information
	int index = 1;
	short tile;

	//Insert everything into the tiles matrix
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			//Grab the specific tile number from the arguments and place it into the start state
			tile=atoi(argv[index++]);
			(*start_state)->tiles[i][j] = tile;

			//If we found the zero tile, update the zero row and column
			if(tile == 0){
				(*start_state)->zero_row = i;
				(*start_state)->zero_column = j;
			}
		}
	}

	//Initialize everything else in the start state
	(*start_state)->total_cost = 0;
	(*start_state)->current_travel = 0;
	(*start_state)->heuristic_cost = 0;
	(*start_state)->next = NULL;
	//Important -- must have no predecessor(root of search tree)
	(*start_state)->predecessor = NULL;

	//Print to the console for the user
	printf("\nInitial state\n");
	print_state(*start_state, N);


	/* Now we create the goal state */	
	
	//Create the goal state itself
	*goal_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate the memory needed in the goal_state
	initialize_state(goal_state, N);	

	int row, col;
	//To create the goal state, place the numbers 1-15 in the appropriate locations
	for(short num = 1; num < N * N; num++){
		//We can mathematically find row and column positions for inorder numbers
		row = (num - 1) / N;
		col = (num - 1) % N;
		(*goal_state)->tiles[row][col] = num;
	}

	//0 is always at the last spot in the goal state
	(*goal_state)->tiles[N-1][N-1] = 0;

	//Initialize everything else in the goal state
	(*goal_state)->zero_row = (*goal_state)->zero_column = N-1;
	(*goal_state)->total_cost = 0;
	(*goal_state)->current_travel = 0;
	(*goal_state)->heuristic_cost = 0;
	(*goal_state)->next=NULL;

	//Print to the console for the user
	printf("Goal state\n");
	print_state(*goal_state, N);
}


/**
 * A simple helper function that allocates memory for closed
 */
void initialize_closed(){
	closed = (struct state**)malloc(sizeof(struct state*) * closed_max_size);
}

void merge_to_closed(struct state* statePtr){
	//If we run out of space, we can expand
	if(next_closed_index == closed_max_size){
		//Double closed max size
		closed_max_size *= 2;
		//Reallocate space for closed
		closed = (struct state**)realloc(closed, sizeof(struct state*) * closed_max_size);
	}

	//Put curr_state into closed
	closed[next_closed_index] = statePtr; 
	//Keep track of the next closed index
	next_closed_index++;

}

/**
 * States will be merged into the fringe linked list in a priority queue fashion by their total_cost values. The lower the value,
 * the higher the priority. This function merges the state pointed to by i into the fringe.
 * Note: This function assumes that succ_states[i] is NOT NULL, must be checked by caller
 */
void priority_queue_insert(struct state* statePtr){
	//Initialize the cursor to be the start of the fringe
	struct state* cursor = fringe;

	//Grab the priority for convenience
	int priority = (statePtr)->total_cost;

	//Special case: inserting at the head
	if(cursor == NULL || priority < cursor->total_cost){
		//Set the succ_states[i] to point to the old head(fringe)
		(statePtr)->next = fringe;
		//Set fringe to the succ_states[i]
		fringe = statePtr;
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
		cursor->next = statePtr;
	} else {
		//Otherwise, we have to break the linked list and insert succ_states[i] into the right spot
		//Save the next state
		struct state* temp = cursor->next;
		//Insert succ_states[i] after cursor
		cursor->next = statePtr;
		//Reattach the rest of the linked list
		(statePtr)->next = temp;
	}

}


struct state* dequeue(){
	struct state* dequeued = fringe;
	fringe = fringe->next;
	return dequeued;
}


int fringe_empty(){
	return fringe == NULL;
}


/**
 * Check to see if the state at position i in the fringe is repeating. If it is, free it and
 * set the pointer to be null
 */
void check_repeating_fringe(struct state* statePtr, const int N){ 	
	//If succ_states[i] is NULL, no need to check anything
	if(statePtr == NULL){
		return;
	}

	//Get a cursor to iterate over the linkedList	
	struct state* cursor = fringe;
	//Go through the linkedList, if we ever find an element that's the same, break out and free the pointer
	while(cursor != NULL){
		//If the states match, we free the pointer and exit the loop
		if(states_same(statePtr, cursor, N)){
			//Properly tear down the dynamic array in the state to avoid memory leaks
			destroy_state(statePtr, N);
			//Free the pointer to the state
			free(statePtr);
			//Set the pointer to be null as a warning
			statePtr = NULL;
			break;
		}
		//Move to the next state in the linkedList
		cursor = cursor->next;
		//Once we get here, the state at i either survived and isn't null, or was freed and set to NULL
	}
}


/**
 * Check for repeats in the closed array. Since we don't need any priority queue functionality,
 * using closed as an array is a major speedup for us
 */
void check_repeating_closed(struct state* statePtr, const int N){
	//If this has already been made null, simply return
	if(statePtr == NULL){
		return;
	}

	//Go through the entire populated closed array
	for(int i = next_closed_index - 1; i > -1; i--){
		//If at any point we find that the states are the same
		if(states_same(closed[i], statePtr, N)){
			//Free both the internal memory and the state pointer itself
			destroy_state(statePtr, N);
			free(statePtr);
			//Set to null as a warning
			statePtr = NULL;
			//Break out of the loop and exit
			break;
		}
	}
	//If we get here, we know that the state was not repeating
}


/**
 * This function simply iterates through successors, passing the appropriate states along to priority_queue_insert if the pointers
 * are not null
 */
int merge_to_fringe(struct state* successors[4]){ 
	//Keep track of how many valid(not null) successors that we merge in
	int valid_successors = 0;

	//Iterate through succ_states, if the given state is not null, call the priority_queue_insert function on it
	for(int i = 0; i < 4; i++){
		if(successors[i] != NULL){
			//If it isn't null, we also know that we have one more unique config, so increment our counterS
			valid_successors++;
			//Insert into queue
			priority_queue_insert(successors[i]);
		}
	}

	return valid_successors;
}
