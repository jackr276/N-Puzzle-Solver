/**
 * Author: Jack Robbins
 * This program implements an A* search algorithm to find the shortest solve path for the 15-puzzle problem
 * game. It takes in a 15-puzzle problem starting configuration in row-major order as a command line argument,
 * and prints out the full solution path to the problem, step by step, if such a solution exists.
 *
 * NOTE: This is the multi-threaded version of the solver, using pthreads
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//For timing
#include <time.h>
//For multi-threading functionality
#include <pthread.h>

//Grid is 4 by 4, 16 tiles total
#define N 4


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


/**
 * Define a structure for holding all thread parameters. We will pass a pointer to a structure of
 * this type into each thread
 */
struct thread_params {
	//For generating successors, the predecessor is needed
	struct state* predecessor;
	//0 = left move, 1 = right move, 2 = down move, 3 = up move
	int option;	
};


/* The following global variables are defined for convenience */
struct state* start_state;
struct state* goal_state;
//The fringe is the set of all states open for exploration. It is maintained as a linked list
struct state* fringe = NULL;
//Closed is a linked list containing all sets previously examined. This is used to avoid repeating
struct state* closed = NULL;
//Every time a state is expanded, at most 4 successor states will be created
struct state* succ_states[4];
/* ========================================================== */


/**
 * Prints out a state by printing out the positions in the 4x4 grid
 */
void print_state(struct state* statePtr) {
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
 * state. It also initializes the goal state mathematically, as it is always the same
 * Note: Assumes a correct number of command line arguments(16 numbers), must be checked by caller
 */
void initialize_start_goal(char** argv){
	/* Begin by creating the start state */
	start_state=(struct state*)malloc(sizeof(struct state));

	//Start at 1, argv[0] is program name
	int index = 1;
	int tile;

	//Insert everything into the tiles matrix
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			//Grab the specific tile number from the arguments and place it into the start state	
			tile=atoi(argv[index++]);
			start_state->tiles[i][j]=tile;

			//If we found the zero tile, update the zero row and column
			if(tile==0){
				start_state->zero_row = i;
				start_state->zero_column = j;
			}
		}
	}

	//Initialize everything else in the start state
	start_state->total_cost=0;
	start_state->current_travel=0;
	start_state->heuristic_cost=0;
	start_state->next=NULL;
	//Important -- must have no predecessor(root of search tree)
	start_state->predecessor=NULL;

	//Print to the console for the user
	printf("Initial state\n");
	print_state(start_state);


	/* Now we create the goal state */	
	goal_state=(struct state*)malloc(sizeof(struct state));
	
	int row, col;
	//To create the goal state, place the numbers 1-15 in the appropriate locations
	for(int num = 1; num < N*N; num++){
		//We can mathematically find row and column positions for inorder numbers
		row = (num - 1) / N;
		col = (num - 1) % N;
		goal_state->tiles[row][col] = num;
	}

	//0 is always at the last spot in the goal state
	goal_state->tiles[N-1][N-1]=0;

	//Initialize everything else in the goal state
	goal_state->total_cost=0;
	goal_state->current_travel=0;
	goal_state->heuristic_cost=0;
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

	/**
	* For heuristic_cost, we will use the manhattan distance from each tile to where it should be.
	* Conveniently, each tile 1-15 should be in position 0-14, so we can find manhattan distance by
	* doing the sum of the absolute difference in coordinates from a number's current state position
	* to its goal state position
	*/

	//Declare all needed variables
	int selected_num, goal_rowCor, goal_colCor;	
	
	//Go through each tile in the state and calculate the heuristic_cost
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
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
 * A simple helper function that will tell if two states are the same. To be used for filtering
 */
int states_same(struct state* a, struct state* b) {
	//Utilize memcmp function on the tiles for convenience
	if (memcmp(a->tiles, b->tiles, sizeof(int) * N*N) == 0){
		//Return 1 if they are the same, 1 corresponds to true
		return 1;	
	}
	//Return 0 if different	
	return 0;
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
			//Free the duplicate state
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
 * This worker thread function generates and checks the validity of a successor that is made by 
 * moving up, down, left or right based on the option given. It will also update the prediction function
 * of the successor if the successor is valid
 */
void* generator_worker(void* thread_params){
	//Initialize a state pointer to be null by default
	struct state* moved = NULL;	
	//Make an appropriate cast to the parameter input struct
	struct thread_params* parameters = (struct thread_params*)thread_params;
	//Grab the option for convenience
	int option = parameters->option;

	//Perform a left move if option is 0 and if possible
	if(option == 0 && parameters->predecessor->zero_column > 0){
		//Create the new state
		moved = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy from predecessor to successor
		copyState(parameters->predecessor, moved);
		//Use helper function to move left
		move_left(moved);

	//Perform a right move if option is 1 and if possible
	} else if(option == 1 && parameters->predecessor->zero_column < N-1){
		//Create the new state
		moved = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy from predecessor to successor
		copyState(parameters->predecessor, moved);
		//Use helper function to move right 
		move_right(moved);

	//Perform a down move if option is 2 and if possible
	} else if(option == 2 && parameters->predecessor->zero_row < N-1){
		//Create the new state
		moved = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy from predecessor to successor
		copyState(parameters->predecessor, moved);
		//Use helper function to move down	
		move_down(moved);

	//Perform an up move if option is 3 and if possible
	} else if(option == 3 && parameters->predecessor->zero_row > 0){
		//Create the new state
		moved = (struct state*)malloc(sizeof(struct state));
		//Perform a deep copy from predecessor to successor
		copyState(parameters->predecessor, moved);
		//Use helper function to move up	
		move_up(moved);
	}
	
	//Whether it's null or not, place the pointer into moved
	succ_states[option] = moved;

	//Only perform the checks if moved is not null
	if(moved != NULL){
		//Now we must check for repeating
		check_repeating(option, fringe);
		check_repeating(option, closed);
		//Update prediction function
		update_prediction_function(option);
	}

	//Threadwork done, no return value will be used
	return NULL;
}


/**
 * This multi-threaded version of successor generation and validation spawns an individual thread for
 * each of the 4 possible moves, potentially expediting the process of generating and checking successors
 */
void generate_valid_successors(struct state* predecessor){
	//We will create 4 threads, once for each successor potential successor
	pthread_t thread_arr[4];
	//We also need 4 thread_param structures 
	struct thread_params* param_arr[4];

	//Create all 4 threads
	for(int i = 0; i < 4; i++){
		//Reserve space and allocate appropriate values in each thread_param
		param_arr[i] = (struct thread_params*)malloc(sizeof(struct thread_params));
		param_arr[i]->predecessor = predecessor;
		//The option will tell the thread function what move to make
		param_arr[i]->option = i;
		
		//Spawn our worker threads, generator_worker is the thread funtion, and paramArr[i]
		//is the needed struct input
		pthread_create(&thread_arr[i], NULL, generator_worker, param_arr[i]);
	}

	//rejoin all the threads
	for(int i = 0; i < 4; i++){
		pthread_join(thread_arr[i], NULL);
		//Free the memory from the parameter structure
		free(param_arr[i]);
	}
}


/**
 * Use an A* search algorithm to solve the 15-puzzle problem by implementing the A* main loop. If the solve function 
 * is successful, it will print the resulting solution path to the console as well.  
 */
int solve(){
	//We will keep track of the time taken to execute
	clock_t begin_CPU = clock();

	//We will keep track of the number of iterations as a sanity check for large problems
	int iter = 0;
	//Put the start state into the fringe to begin the search
	fringe = start_state; 
	//Maintain a pointer for the current state in the search
	struct state* curr_state;

	//Algorithm main loop -- while there are still states to be expanded, keep iterating until we find a solution
	while (fringe != NULL) {
		//Remove or "pop" the head of the fringe linked list -- because fringe is a priority queue, this is the most
		//promising state to explore next
		curr_state=fringe;
		fringe=fringe->next;

		//Check to see if we have found the solution. If we did, we will print out the solution path and stop
		if(states_same(curr_state, goal_state)){
			//Stop the clock if we find a solution
			clock_t end_CPU = clock();

			//Determine the time spent on the CPU
			double time_spent_CPU = (double)(end_CPU - begin_CPU) / CLOCKS_PER_SEC;

			//Now find the solution path by working backwords
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
			//Print out the time taken to solve	
			printf("\nSolution found!\nTotal CPU time spent: %.7f seconds\n", time_spent_CPU);

			//Print out solution path
			printf("\nNow displaying solution path\n");
			//Display the path length for the user
			printf("Path Length: %d\n", pathlen); 

			//Print out the solution path in order
			while(solution_path != NULL){
				print_state(solution_path);
				solution_path = solution_path->next;
			}	

			//We've found a solution, so the function should exit 
			return 0;	
		}
		
		/**
		 * This part can benefit from multi-threading, generate_valid_successors implements
		 * multiple threads for creating and checking the validity of successors
		 */

		//Generate successors to the current state once we know it isn't a solution
		generate_valid_successors(curr_state);
		
		/* End multi-threading */

		//Add all necessary states to fringe now that we have checked for repeats and updated predictions 
		merge_to_fringe(); 
		
		//Move the current state into closed, insert at head("Push")
		curr_state->next=closed;
		//Maintain closed properly
		closed=curr_state;

		//For very complex problems, print the iteration count to the console for a sanity check
		if(iter > 1 && iter % 1000 == 0) {
			printf("Still calculating, on %dth iteration\n", iter);
		}
		
		//End of one full iteration
		iter++;
	}
	
	//If we end up here, fringe became NULL with no goal configuration found, so there is no solution
	printf("No solution.");
	return 0;
}


/**
 * The main function simply makes the needed calls to the initialize and solve function after checking command
 * line arguments
 */
int main(int argc, char** argv) {
	//Check if the number of arguments is correct. If not, exit the program and print an error
	if(argc != 17){
		//Give an error message
		printf("Incorrect number of program arguments. Please retry with a correct configuration.\n");
		return 1;
	}

	//Initialize the goal and start states 
	initialize_start_goal(argv);
	//Call the solve() funciton and hand off the rest of the program execution to it
	return solve();
}
