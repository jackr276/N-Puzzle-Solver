/**
 * Author: Jack Robbins 
 * This program implements an A* search algorithm to find the shortest solve path for the 15-puzzle problem game. 
 * It takes in an N-puzzle problem starting configuration in row-major order as a command line argument, following a number N for the
 * NxN size of the puzzle and prints out the full solution path to the problem, step by step, if such a solution exists.
 *
 * Note: This is the single-threaded version of the solver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//For timing
#include <time.h>
#include "puzzle.h"


/**
 * States will be merged into the fringe linked list in a priority queue fashion by their total_cost values. The lower the value,
 * the higher the priority. This function merges the state pointed to by i into the fringe.
 * Note: This function assumes that succ_states[i] is NOT NULL, must be checked by caller
 */
void priority_queue_insert(struct state* statePtr){
	//Initialize the cursor to be the start of the fringe
	struct state* cursor = fringe;

	//Grab the priority for convenience
	int priority = statePtr->total_cost;

	//Special case: inserting at the head
	if(cursor == NULL || priority < cursor->total_cost){
		//Set the succ_states[i] to point to the old head(fringe)
		statePtr->next = fringe;
		//Set fringe to the succ_states[i]
		fringe = statePtr; //Exit once done
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
		statePtr->next = temp;
	}
}


/**
 * Update the prediction function for the state pointed to by succ_states[i]. If this pointer is null, simply skip updating
 * and return. This is a generic algorithm, so it will work for any size N
 */ 
void update_prediction_function(struct state* statePtr, int N){
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
	short selected_num, goal_rowCor, goal_colCor;
	//Keep track of the manhattan distance
	int manhattan_distance;
	
	//Go through each tile in the state and calculate the heuristic_cost
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			//grab the number to be examined
			selected_num = statePtr->tiles[i][j];

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
			statePtr->heuristic_cost += manhattan_distance;
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
			left = statePtr->tiles[i][j];

			//If this tile is 0, it's irrelevant so do not explore further
			if(left == 0){
				continue;
			}
			
			//Now go through every tile in the row after left, this is what makes this generalized linear conflict
			for(int k = j+1; k < N; k++){
				//Grab right tile for convenience
				right = statePtr->tiles[i][k];

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
			above = statePtr->tiles[i][j];

			//If this tile is 0, it's irrelevant so do not explore further
			if(above == 0){
				continue;
			}

			//Now go through every tile in the column below "above", this is what makes it generalized linear conflict
			for(int k = i+1; k < N; k++){
				//Grab the below tile for convenience
				below = statePtr->tiles[k][j];

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
	statePtr->heuristic_cost += linear_conflicts * 2;

	//Once we have the heuristic_cost, update the total_cost
	statePtr->total_cost = statePtr->heuristic_cost + statePtr->current_travel;
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
		copy_state(predecessor, leftMove);
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
		copy_state(predecessor, rightMove);
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
		copy_state(predecessor, downMove);
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
		copy_state(predecessor, upMove);
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
 * Check to see if the state at position i in the fringe is repeating. If it is, free it and
 * set the pointer to be null
 */
void check_repeating_fringe(int i){ 	
	//If succ_states[i] is NULL, no need to check anything
	if(succ_states[i] == NULL){
		return;
	}

	//Get a cursor to iterate over the linkedList	
	struct state* cursor = fringe;
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
 * Check for repeats in the closed array. Since we don't need any priority queue functionality,
 * using closed as an array is a major speedup for us
 */
void check_repeating_closed(int max_index, int state_index){
	//If this has already been made null, simply return
	if(succ_states[state_index] == NULL){
		return;
	}

	//Go through the entire populated closed array
	for(int i = max_index - 1; i > -1; i--){
		//If at any point we find that the states are the same
		if(states_same(closed[i], succ_states[state_index])){
			//Free both the internal memory and the state pointer itself
			destroy_state(succ_states[state_index]);
			free(succ_states[state_index]);
			//Set to null as a warning
			succ_states[state_index] = NULL;
			//Break out of the loop and exit
			break;
		}
	}
	//If we get here, we know that the state was not repeating
}


/**
 * Use an A* search algorithm to solve the 15-puzzle problem by implementing the A* main loop. If the solve function 
 * is successful, it will print the resulting solution path to the console as well.  
 */
int solve(int* N){
	//Get the CPU clock start time
	clock_t begin_CPU = clock();

	//We will keep track of the number of iterations as a sanity check for large problems
	int iter = 0;
	//Keep track of the number of unique configurations made
	int num_unique_configs = 0;
	//Put the start state into the fringe to begin the search
	fringe = start_state; 
	//We will keep a reference to the next available closed index	
	int next_closed_index = 0;
	//Make the initial allocation for closed
	closed = malloc(sizeof(struct state*) * closed_max_size);

	//Maintain a pointer for the current state in the search
	struct state* curr_state;

	struct state* succ_states[4];

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
				print_state(solution_path, N);
				solution_path = solution_path->next;
			}	

			//Print out all running statistics
			printf("------------- Program Running Statistics -------------\n\n");
			//Print out the path length
			printf("Optimal solution path length: %d\n", pathlen);
			//Print out the number of unique configurations generated
			printf("Unique configurations generated by solver: %d\n", num_unique_configs);
			//Print out total memory consumption in Megabytes
			printf("Memory consumed: %.2f MB\n", (sizeof(struct state) + N*N*sizeof(short)) * num_unique_configs / 1048576.0);
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
			//If the state is already null, there is no point in further exploration
			if(succ_states[i] == NULL){
				continue;
			}

			//Check each successor state against fringe and closed to see if it is repeating
			//Check the current state in the closed array
			check_repeating_closed(next_closed_index, i);
			//Check against fringe
			check_repeating_fringe(i); 
			//Update the prediction function on states that don't repeat
			update_prediction_function(i); 
		}
	
		//Add all necessary states to fringe now that we have checked for repeats and updated predictions 
		//Additionally, we need to update the num_unique_configs, this will be done in merge_to_fringe 
		num_unique_configs += merge_to_fringe(succ_states); 

		//Add to closed
		//If we run out of space, we can expand
		if(next_closed_index == closed_max_size){
			//Double closed max size
			closed_max_size *= 2;
			//Reallocate space for closed
			closed = realloc(closed, sizeof(struct state*) * closed_max_size);
		}

		//Put curr_state into closed
		closed[next_closed_index] = curr_state;
		//Keep track of the next closed index
		next_closed_index++;

		//For very complex problems, print the iteration count to the console for a sanity check
		if(iter > 1 && iter % 1000 == 0) {
			printf("Iteration: %6d, %6ld total unique states generated\n", iter, num_unique_configs);
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
	//The size of our N puzzle
	int N;

	//If the user put in a non-integer or nonpositive integer, print an error
	if(sscanf(argv[1], "%d", &N) != 1 || N < 1){
		printf("Incorrect type of program arguments.\n");
		printf("Usage: ./solve <N> <n0. . .nN>\n");
		printf("Where <N> is the number of rows/columns, followed by the matrix in row-major order.\n\n");
		return 1;
	}

	//Check if the number of arguments is correct. If not, exit the program and print an error
	if(argc != N*N + 2){
		//Give an error message
		printf("Incorrect number of program arguments.\n");
		printf("Usage: ./solve <N> <n0. . .nN>\n");
		printf("Where <N> is the number of rows/columns, followed by the matrix in row-major order.\n\n");
		return 1;
	}

	//Important: Move the address of argv up by 1 so that initialize_start_goal can only see the initial config 
	argv += 1;

	//Initialize the goal and start states 
	initialize_start_goal(argv, &N);

	//Call the solve() funciton and hand off the rest of the program execution to it
	return solve(&N);
}
