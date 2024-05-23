/**
 * Author: Jack Robbins This program implements an A* search algorithm to find the shortest solve path for the 15-puzzle problem game. 
 * It takes in an N-puzzle problem starting configuration in row-major order as a command line argument, following a number N for the
 * NxN size of the puzzle and prints out the full solution path to the problem, step by step, if such a solution exists.
 *
 * Note: This version of the solver makes use of disjoint pattern databases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//For timing
#include <time.h>


/*=================================== Global Variables and Structures ============================================================ */


/**
 * Define a type of state, which is a structure, that represents a configuration in the gem puzzle game
 */
struct state {
	//Define a dynamic 2D array for the tiles since we have a variable puzzle size
	int** tiles;
	//For A*, define the total_cost, how far the tile has traveled, and heuristic cost int total_cost, current_travel, heuristic_cost;
	int total_cost, current_travel, heuristic_cost;
	//location (row and colum) of blank tile 0
	short zero_row, zero_column;
	//The next state in the linked list(fringe or closed), NOT a successor
	struct state* next;
	//The predecessor of the current state, used for tracing back a solution	
	struct state* predecessor;			
};


//Define a self referential struct to store a pattern and a cost. The pattern is what we really care about here
struct pattern_cost{
	int* pattern;
	int pattern_length;
	int cost;
	//0 = first half, 1  = last half
	int pattern_type;
	struct pattern_cost* next;
};


//N is the NxN size of the puzzle, defined by the user
int N;
//Keep track of how many unique configs we've created
long num_unique_configs = 0;
//Keep track of our database hit rate
int num_database_hits = 0;
//The starting and goal states
struct state* start_state;
struct state* goal_state;
//The fringe is the set of all states open for exploration. It is maintained as a linked list
struct state* fringe = NULL;
//Closed is a linked list containing all sets previously examined. This is used to avoid repeating
struct state* closed = NULL;
//Keep track of patterns from the database
struct pattern_cost* patterns_first_half = NULL;
struct pattern_cost* patterns_last_half = NULL;
//Every time a state is expanded, at most 4 successor states will be created
struct state* succ_states[4];
/* ===================================================================================================================================== */


/**
 * Before we can do anything, we must read the entire pattern database into memory as two big linked lists
 */
void read_pattern_db(FILE* database){
	//We will use the new_node as our holder for reading the database in
	struct pattern_cost* new_node;
	int MAXLENGTH = 1000;
	int num_patterns_loaded = 0;

	//Declare lines, tokens and buffer sizes for line reading
	char line[1000];
	char* token;

	//Grab each line in the file
	while(fgets(line, MAXLENGTH,  database) != NULL)	{
		//Reserve space for the new node
		new_node = malloc(sizeof(struct pattern_cost));

		//Grab tokens delimited by spaces
		token = strtok(line, " ");
		
		//The first integer is always the pattern type
		//Read the token as an int and place it in pattern_type
		sscanf(token, "%d", &new_node->pattern_type);		

		//Use the pattern length to allocate enough space
		if(!new_node->pattern_type){
			new_node->pattern_length = N * N / 2;	
			//We may as well attach to the linked list while we're here
			new_node->next = patterns_first_half;
			//Set the head to be the new node
			patterns_first_half = new_node;
		} else {
			new_node->pattern_length = (N * N / 2) - 1;
			//We may as well attach to the linked list while we're here
			new_node->next = patterns_last_half;
			//Set the head to be the new node
			patterns_last_half = new_node;
		}

		//Reserve space for the pattern
		new_node->pattern = calloc(sizeof(int), new_node->pattern_length);

		//Get the next token
		token = strtok(NULL, " ");

		//The cost of the pattern always comes next
		sscanf(token,  "%d", &new_node->cost);

		//Now we read in the pattern int by int
		for(int i = 0; i < new_node->pattern_length; i++){
			//Get the next token
			token = strtok(NULL, " ");
			//Place it into the pattern array
			sscanf(token, "%d", new_node->pattern + i);
		}

		//We've loaded one more pattern
		num_patterns_loaded++;
	}

	printf("\nSuccessfully loaded %d patterns into memory\n", num_patterns_loaded);
}


/**
 * The initialize_state function takes in a pointer to a state and reserves the appropriate space for the dynamic array
 * that holds the tiles 
 */
void initialize_state(struct state* statePtr){
	//Declare all of the pointers needed for each row
	statePtr->tiles = malloc(sizeof(int*) * N);

	//For each row, allocate space for N integers
	for(int i = 0; i < N; i++){
		statePtr->tiles[i] = malloc(sizeof(int) * N);
	}
}


/**
 * The destroy_state function does the exact reverse of the initialize_state function to properly free memory
 */
void destroy_state(struct state* statePtr){
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
void print_state(struct state* statePtr){
	//Go through tile by tile and print out
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++){
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
 * The initialization function takes in the command line arguments and translates them into the initial
 * state. It also initializes the goal state mathematically, as it is always the same
 * Note: Assumes a correct number of command line arguments(16 numbers), must be checked by caller
 */
void initialize_start_goal(char** argv){
	/* Begin by creating the start state */

	//Create the start state itself
	start_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate memory needed in the start_state
	initialize_state(start_state);

	//Start at 1, argv[0] is program name and argv has been adjusted up by 1 to only contain the start state information
	int index = 1;
	int tile;

	//Insert everything into the tiles matrix
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			//Grab the specific tile number from the arguments and place it into the start state
			tile=atoi(argv[index++]);
			start_state->tiles[i][j] = tile;

			//If we found the zero tile, update the zero row and column
			if(tile == 0){
				start_state->zero_row = i;
				start_state->zero_column = j;
			}
		}
	}

	//Initialize everything else in the start state
	start_state->total_cost = 0;
	start_state->current_travel = 0;
	start_state->heuristic_cost = 0;
	start_state->next = NULL;
	//Important -- must have no predecessor(root of search tree)
	start_state->predecessor = NULL;

	//Print to the console for the user
	printf("\nInitial state\n");
	print_state(start_state);


	/* Now we create the goal state */	
	
	//Create the goal state itself
	goal_state = (struct state*)malloc(sizeof(struct state));
	//Dynamically allocate the memory needed in the goal_state
	initialize_state(goal_state);	

	int row, col;
	//To create the goal state, place the numbers 1-15 in the appropriate locations
	for(int num = 1; num < N*N; num++){
		//We can mathematically find row and column positions for inorder numbers
		row = (num - 1) / N;
		col = (num - 1) % N;
		goal_state->tiles[row][col] = num;
	}

	//0 is always at the last spot in the goal state
	goal_state->tiles[N-1][N-1] = 0;

	//Initialize everything else in the goal state
	goal_state->total_cost = 0;
	goal_state->current_travel = 0;
	goal_state->heuristic_cost = 0;
	goal_state->next=NULL;

	//Print to the console for the user
	printf("Goal state\n");
	print_state(goal_state);
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
		fringe = succ_states[i]; //Exit once done
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
void merge_to_fringe(){ 
	//Iterate through succ_states, if the given state is not null, call the priority_queue_insert function on it
	for(int i = 0; i < 4; i++){
		if(succ_states[i] != NULL){
			//If it isn't null, we also know that we have one more unique config, so increment our counter
			num_unique_configs++;
			//Insert into queue
			priority_queue_insert(i);
		}
	}	
}

/**
 * Takes in a state and translates/compresses the 2D array into a 1D array with only the information that we care about
 */
void generate_pattern_from_state(struct pattern_cost* patternPtr, struct state* statePtr){
	int tile;

	int pattern_type = patternPtr->pattern_type;

	//Determine how much space needs to be allocated for the pattern
	if(!pattern_type){
		patternPtr->pattern_length = N * N / 2;	
	} else {
		patternPtr->pattern_length = (N * N / 2) - 1;
	}
	
	patternPtr->pattern = calloc(sizeof(int), patternPtr->pattern_length);
	
	//Generation for the first pattern type
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//If we have a pattern tile
			if((tile = statePtr->tiles[i][j]) != 0){	
				if(!pattern_type && tile <= N*N/2){
					//We store the index of the tile in the pattern at tile location - 1
					//Example: If tile 1 is at position 8, we store [8] in the 0 spot of the array
					patternPtr->pattern[tile - 1] = i * N + j;
				} else if(pattern_type && tile > N*N/2){
					//9 should be at 0 spot
					patternPtr->pattern[tile - (N*N / 2) - 1] = i * N + j;	
				}
			}
		}
	}	
}


/**
 * Define a simple function that checks if two patterns are identical for use in contained_in_db
 */
int patterns_same(int* pattern1, int* pattern2, int length){
	//Compare tile by tile
	for(int i = 0; i < length; i++){
		if(pattern1[i] != pattern2[i]){
			return 0;
		}	
	}
	
	//Return 1 because they are the same
	return 1;
}


/**
 * Traverse the appropriate database linked list and return the precomputed cost of a given
 * pattern, if we can find one
 */
int get_cost_from_db(struct pattern_cost* patternPtr){
	struct pattern_cost* cursor;

	//Set the cursor to be the appropriate linked list
	if(!patternPtr->pattern_type){
		cursor = patterns_first_half;
	}else{
		cursor = patterns_last_half;
	}

	while(cursor != NULL){
		//If the patterns match, return the cost of the cursor
		if(patterns_same(patternPtr->pattern, cursor->pattern, patternPtr->pattern_length)){
			num_database_hits++;
			return cursor->cost;
		}
	
		//Advance the cursor to the next node
		cursor = cursor->next;
	}

	//If we get here, there was no match so return 0 for the cost
	return 0;
}


/**
 * Update the prediction function for the state pointed to by succ_states[i]. If this pointer is null, simply skip updating
 * and return. This is a generic algorithm, so it will work for any size N
 */ 
void update_prediction_function(int i){
	struct state* statePtr = succ_states[i];
	//If statePtr is null, this state was a repeat and has been freed, so don't calculate anything
	if(statePtr == NULL){
		return;
	}

	//The current_travel of the state has already been updated by stateCopy, so we only need to find the heuristic_cost
	statePtr->heuristic_cost = 0;

	/**
	* For heuristic_cost, we will use the manhattan distance from each tile to where it should be.
	* Conveniently, each tile 1-15 should be in position 0-14, so we can find manhattan distance by
	* doing the sum of the absolute difference in coordinates from a number's current state position
	* to its goal state position
	*/

	//Declare all needed variables
	int selected_num, goal_rowCor, goal_colCor;
	//Keep track of the manhattan distance
	int manhattan_distance = 0;
	
	//Go through each tile in the state and calculate the heuristic_cost
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//grab the number to be examined
			selected_num = statePtr->tiles[i][j];

			//We do not care about 0 as it can move, so skip it
			if(selected_num == 0){
				continue;
			//Otherwise mathematically find the needed position
			} else {
				//Goal row coordinate is the index of the number divided by number of rows
				goal_rowCor = (selected_num - 1) / N;
				//Goal column coordinate is the index modulated by column length 
				goal_colCor = (selected_num - 1) % N;
			}

			//Manhattan distance is the absolute value of the x distance and the y distance
			manhattan_distance = abs(i - goal_rowCor) + abs(j - goal_colCor);	
		
			//Add manhattan distance for each tile
			statePtr->heuristic_cost += manhattan_distance;
		}
	}

	/**
	 * We will now attempt to find pattern matches for this state in the precomputed pattern database, which
	 * has already been read into memory. To do this, we'll need to convert our state into a pattern, and
	 * then traverse the database to see if there are any matches
	 */

	int database_cost;
	//Decare 2 patterns that we will be using
	struct pattern_cost* first_half = malloc(sizeof(struct pattern_cost));
	struct pattern_cost* last_half = malloc(sizeof(struct pattern_cost));
	//Set the pattern_type appropriately
	first_half->pattern_type = 0;
	last_half->pattern_type = 1;
	
	//Get the pattern from the state that we are evaluating
	generate_pattern_from_state(first_half, statePtr);
	generate_pattern_from_state(last_half, statePtr);

	statePtr->heuristic_cost += get_cost_from_db(first_half) + get_cost_from_db(last_half);
	
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
		//Dynamically allocate the memory needed in leftMove
		initialize_state(leftMove);
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
		//Dynamically allocate the memory needed in rightMove
		initialize_state(rightMove);
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
		//Dynamically allocate the memory needed in downMove
		initialize_state(downMove);
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
		//Dynamically allocate the memory needed in upMove
		initialize_state(upMove);
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
int states_same(struct state* a, struct state* b){
	//Go through each row in the dynamic tile matrix in both states
	for(int i = 0; i < N; i++){
		//We can use memcmp to efficiently compare the space pointed to by each pointer
		if (memcmp(a->tiles[i], b->tiles[i], sizeof(int) * N) != 0){
			//If we find a difference, return 0
			return 0;
		}
	}

	//Return 1 if same	
	return 1;
}


/**
 * Check to see if the state at position i in the given linkedList is repeating. If it is, free it and
 * set the pointer to be null
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
			//Properly tear down the dynamic array in the state to avoid memory leaks
			destroy_state(succ_states[i]);
			//Free the pointer to the state
			free(succ_states[i]);
			//Set the pointer to be null as a warning
			succ_states[i] = NULL;
			break;
		}
		//Move to the next state in the linkedList
		cursor = cursor->next;
		//Once we get here, the state at i either survived and isn't null, or was freed and set to NULL
	}
}


/**
 * Use an A* search algorithm to solve the 15-puzzle problem by implementing the A* main loop. If the solve function 
 * is successful, it will print the resulting solution path to the console as well.  
 */
int solve(){
	//Get the CPU clock start time
	clock_t begin_CPU = clock();

	//We will keep track of the number of iterations as a sanity check for large problems
	int iter = 0;
	//Put the start state into the fringe to begin the search
	fringe = start_state; 
	//Maintain a pointer for the current state in the search
	struct state* curr_state;


	//Algorithm main loop -- while there are still states to be expanded, keep iterating until we find a solution
	while (fringe != NULL){
		//Remove or "pop" the head of the fringe linked list -- because fringe is a priority queue, this is the most
		//promising state to explore next
		curr_state=fringe;
		fringe=fringe->next;

		//Check to see if we have found the solution. If we did, we will print out the solution path and stop
		if(states_same(curr_state, goal_state)){
			//Stop the clock if we find solution
			clock_t end_CPU = clock();

			//Determine the time spent for CPU time 
			double time_spent_CPU = (double)(end_CPU - begin_CPU) / CLOCKS_PER_SEC;

			//Now find the solution path
			//Keep track of how long the path is	
			int pathlen = 0;
			//Keep a linked list for our solution path
			struct state* solution_path = NULL;
		
			//Put the states into the solution path in reverse order(insert at the head) using their predecessor
			while(curr_state != NULL){
				//Insert the current state at the head of solution path
				curr_state->next = solution_path;
				solution_path = curr_state;
				//Go back up the solution chain using predecessor
				curr_state = curr_state->predecessor;
				//Increment the path length
				pathlen++;
			}
			//Print out the solution path first
			printf("\nSolution found! Now displaying solution path\n");
			//Display the path length for the user
			printf("Path Length: %d\n\n", pathlen); 

			//Print out the solution path in order
			while(solution_path != NULL){
				print_state(solution_path);
				solution_path = solution_path->next;
			}	

			//Print out all running statistics
			printf("------------- Program Running Statistics -------------\n\n");
			//Print out the path length
			printf("Optimal solution path length: %d\n", pathlen);
			//Print out the number of unique configurations generated
			printf("Unique configurations generated by solver: %ld\n", num_unique_configs);
			//Print out total memory consumption in Megabytes
			printf("Memory consumed: %.2f MB\n", (sizeof(struct state) + N*N*sizeof(int)) * num_unique_configs / 1048576.0);
			//Print out CPU time(NOT wall time) spent
			printf("Total CPU time spent: %.7f seconds\n\n", time_spent_CPU);	
			printf("------------------------------------------------------\n\n");

			//We've found a solution, so the function should exit 
			return 0;	
		}

		//Generate successors to the current state once we know it isn't a solution
		generate_successors(curr_state);

		//Go through each of the successor states, and check for repetition/update prediction function
		for(int i = 0; i < 4; i++){
			//Check each successor state against fringe and closed to see if it is repeating
			check_repeating(i,fringe); 
			check_repeating(i,closed);
			//Update the prediction function on states that don't repeat
			update_prediction_function(i); 
		}
	
		//Add all necessary states to fringe now that we have checked for repeats and updated predictions 
		//Additionally, we need to update the num_unique_configs, this will be done in merge_to_fringe 
		merge_to_fringe(); 
		
		//Move the current state into closed, insert at head("Push")
		curr_state->next=closed;
		//Maintain closed properly
		closed=curr_state;

		//For very complex problems, print the iteration count to the console for a sanity check
		if(iter > 1 && iter % 1000 == 0) {
			printf("Iteration: %6d, %6ld total unique states generated, %6d database hits\n", iter, num_unique_configs, num_database_hits);
		}
		
		//End of one full iteration
		iter++;
	}
	
	//If we end up here, fringe became NULL with no goal configuration found, so there is no solution
	printf("No solution.\n");
	return 0;
}


/**
 * The main function simply makes the needed calls to the initialize and solve function after checking command
 * line arguments
 */
int main(int argc, char** argv){
	//If the user put in a non-integer or nonpositive integer, print an error
	if(sscanf(argv[1], "%d", &N) != 1 || N < 1){
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./solve <N> <Database File> <n0. . .nN>\n");
		printf("Where <N> is the number of rows/columns and <Database File> is the pattern database, followed by the matrix in row-major order.\n\n");
		return 1;
	}

	//Check if the number of arguments is correct. If not, exit the program and print an error
	if(argc != N*N + 3){
		//Give an error message
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./solve <N> <Database File> <n0. . .nN>\n");
		printf("Where <N> is the number of rows/columns and <Database File> is the pattern database, followed by the matrix in row-major order.\n\n");
		return 1;
	}

	//Open the file
	FILE* db = fopen(argv[2], "r");

	//If we failed at opening the file, let the user know and exit
	if(db == NULL){
		printf("ERROR. Pattern database %s not found. Program will exit.\n", argv[2]);
		return 1;
	}

	//Read the pattern database in
	read_pattern_db(db);

	//Close when done
	fclose(db);

	printf("Pattern database read successfully! Solver will now begin searching.\n");

	//Important: Move the address of argv up by 2 so that initialize_start_goal can only see the initial config 
	argv += 2;
	
	//Initialize the goal and start states 
	initialize_start_goal(argv);
	//Call the solve() funciton and hand off the rest of the program execution to it
	return solve();
}
