package solver;

import java.util.ArrayList;
import java.util.PriorityQueue;
import java.util.Stack;

import puzzle.Puzzle;
import puzzle.PuzzleComparator;

public class Solver{

	public void solve(Puzzle start, Puzzle goal){
		//Begin timing the solver
		long startTime = System.nanoTime();
		//Keep track of the number of iterations and states generated
		int numIterations = 0;
		int uniqueStates = 0;
		
		//Declare the fringe for holding current states
		PriorityQueue<Puzzle> fringe = new PriorityQueue<>(new PuzzleComparator());
		//Declare the closed list to store explored states
		ArrayList<Puzzle> closed = new ArrayList<>();

		//Put the goal into the fringe to seed the search
		fringe.offer(start);

		//While there are still states to expand
		while(!fringe.isEmpty()){
			//Pop the head off of the queue
			Puzzle current = fringe.poll();

			//If we have the goal, stop searching and print the solution
			if(current.equals(goal)){
				//Stop the timer
				long endTime = System.nanoTime();
				//Display solution
				printSolution(current);
				//Print out runtime stats
				System.out.println("--------------------------------------------------");
				System.out.printf("Time taken to solve: %7f",  endTime - startTime / 1000000000.0);
				System.out.println(uniqueStates + " unique states generated");
				System.out.println("--------------------------------------------------");
				return;
			}

			//Generate the successors from current 
			ArrayList<Puzzle> successors = generateSuccessors(current);

			//Check for repeats and add into fringe
			for(Puzzle successor : successors){
				//If the puzzle is not a repeat, update the prediction function and add it to the fringe 
				if(!fringe.contains(successor) && !closed.contains(successor)){
					successor.updatePredictionFunction();
					fringe.offer(successor);
				}
			}

			//Put the current node into closed
			closed.add(current);

			//Print a sanity check to the console every 1000 iterations
			if(numIterations > 0 && numIterations % 1000 == 0){
				System.out.println(numIterations + " iterations, " + uniqueStates + " unique states generated");
			}
			//Increment the number of iterations
			numIterations++;
		}

		//If we ever end up here, there were no solutions
		System.out.println("No solution exists for this puzzle.");
	}


	/**
	 * A private helper method that generates possible successors to the current state.
	 * This is also known as "Expanding"
	 */
	private ArrayList<Puzzle> generateSuccessors(Puzzle current){
		//Define a list to hold our values
		ArrayList<Puzzle> successors = new ArrayList<>();
		
		//If we can actually make the successor, we'll store it here
		Puzzle validSuccessor;

		//There are 4 possible successors, one for each move
		for(int i = 0; i < 4; i++){
			validSuccessor = current.generateSuccessor(i);	
			//If generation worked, save the successor
			if(validSuccessor != null){
				successors.add(validSuccessor);
			}
		}
		
		//Return the list to solve()
		return successors;
	}


	/**
	 * A solution printer that treats a puzzle like a linked list node, tracing back the solution
	 */
	private void printSolution(Puzzle end){
		//We will store the solution in a stack
		Stack<Puzzle> solutionTrace = new Stack<>();
		int pathLength = 0;

		//Trace the solution back up the chain
		while(end != null){
			//Stack ensures FILO
			solutionTrace.push(end);
			end = end.getPredecessor();
			pathLength++;
		}

		//Display path length
		System.out.println("\nSolution found! Path is of length: " + pathLength + "\n");

		Puzzle current;
	
		//Now go backwards through the solution trace, printing each state
		while(!solutionTrace.isEmpty()){
			System.out.println(solutionTrace.pop() + "\n");
		}
	}

}
