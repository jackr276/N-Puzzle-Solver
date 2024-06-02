/**
 * Author: Jack Robbins 
 * This program implements an A* search algorithm to find the shortest solve path for the 15-puzzle problem game. 
 * It takes in an N-puzzle problem starting configuration in row-major order as a command line argument, following a number N for the
 * NxN size of the puzzle and prints out the full solution path to the problem, step by step, if such a solution exists.
 *
 * Note: This is the single-threaded version of the solver
 */

//For timing
#include <time.h>
#include "puzzle.h"


/**
 * This function generates all possible successors to a state and stores them in the successor array
 * Note: 4 successors are not always possible, if a successor isn't possible, NULL will be put in its place 
 */
void generate_successors(struct state* predecessor, struct state** successors, int N){
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
		initialize_state(leftMove, N);
		//Perform a deep copy on the state
		copy_state(predecessor, leftMove, N);
		//Move right by one
		move_left(leftMove);
	}
	//Put leftMove into the array
	successors[0] = leftMove;

	//Generate successor by moving right one if possible
	if(predecessor->zero_column < N-1){
		//Create a new state
		rightMove = (struct state*)malloc(sizeof(struct state));
		//Dynamically allocate the memory needed in rightMove
		initialize_state(rightMove, N);
		//Perform a deep copy on the state
		copy_state(predecessor, rightMove, N);
		//Move right by one
		move_right(rightMove);
	}
	//Put rightMove into the array
	successors[1] = rightMove;
	
	//Generate successor by moving down one if possible
	if(predecessor->zero_row < N-1){
		//Create a new state
		downMove = (struct state*)malloc(sizeof(struct state));
		//Dynamically allocate the memory needed in downMove
		initialize_state(downMove, N);
		//Perform a deep copy on the state
		copy_state(predecessor, downMove, N);
		//Move down by one
		move_down(downMove);
	}
	//Put downMove into the array
	successors[2] = downMove;
	
	//Generate successor by moving down one if possible
	if(predecessor->zero_row > 0){
		//Create a new state
		upMove = (struct state*)malloc(sizeof(struct state));
		//Dynamically allocate the memory needed in upMove
		initialize_state(upMove, N);
		//Perform a deep copy on the state
		copy_state(predecessor, upMove, N);
		//Move up by one
		move_up(upMove);
	}
	//Put upMove into the array
	successors[3] = upMove;
}


/**
 * Use an A* search algorithm to solve the 15-puzzle problem by implementing the A* main loop. If the solve function 
 * is successful, it will print the resulting solution path to the console as well.  
 */
int solve(int N, struct state* start_state, struct state* goal_state){
	//Get the CPU clock start time
	clock_t begin_CPU = clock();

	//We will keep track of the number of iterations as a sanity check for large problems
	int iteration = 0;
	//Keep track of the number of unique configurations made
	int num_unique_configs = 0;
	//Define an array for holding successor states. We can generate at most 4 each time
	struct state* successors[4];

	//Put the start state into the fringe to begin the search
	priority_queue_insert(start_state);

	//Initialize the closed list
	initialize_closed();

	//Maintain a pointer for the current state in the search
	struct state* curr_state;

	//Algorithm main loop -- while there are still states to be expanded, keep iterating until we find a solution
	while (!fringe_empty()){
		//Remove or "pop" the head of the fringe linked list -- because fringe is a priority queue, this is the most
		//promising state to explore next
		curr_state = dequeue();

		//Check to see if we have found the solution. If we did, we will print out the solution path and stop
		if(states_same(curr_state, goal_state, N)){
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
				print_state(solution_path, N, 0);
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
		generate_successors(curr_state, successors, N);

		//Go through each of the successor states, and check for repetition/update prediction function
		for(int i = 0; i < 4; i++){
			//If the state is already null, there is no point in further exploration
			if(successors[i] == NULL){
				continue;
			}

			//Check each successor state against fringe and closed to see if it is repeating
			//Check the current state in the closed array
			check_repeating_closed(&successors[i], N);
			//Check against fringe
			check_repeating_fringe(&successors[i], N);
			//Update the prediction function on states that don't repeat
			update_prediction_function(successors[i], N); 
		}

		//Add all necessary states to fringe now that we have checked for repeats and updated predictions 
		//Additionally, we need to update the num_unique_configs, this will be done in merge_to_fringe 
		num_unique_configs += merge_to_fringe(successors); 

		//Merge the current state into closed
		merge_to_closed(curr_state);

		//For very complex problems, print the iteration count to the console for a sanity check
		if(iteration > 1 && iteration % 1000 == 0) {
			printf("Iteration: %6d, %6d total unique states generated\n", iteration, num_unique_configs);
		}
		
		//End of one full iteration
		iteration++;
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

	//The starting and goal states, must create and reserve space
	struct state* start_state = (struct state*)malloc(sizeof(struct state));
	struct state* goal_state = (struct state*)malloc(sizeof(struct state));

	//Initialize the goal and start states 
	initialize_start_goal(argv, start_state, goal_state, N);

	//Call the solve() funciton and hand off the rest of the program execution to it
	return solve(N, start_state, goal_state);
}
