package solver;

import java.util.ArrayList;
import java.util.PriorityQueue;

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
		fringe.offer(goal);

		//While there are still states to expand
		while(!fringe.isEmpty()){
			//Pop the head off of the queue
			Puzzle current = fringe.poll();

			//If we have the goal, stop searching and print the solution
			if(current.equals(goal)){
				//Stop the timer
				long endTime = System.nanoTime();
				printSolution();
				//Print out runtime stats
				System.out.println("--------------------------------------------------");
				System.out.printf("Time taken to solve: %7f",  endTime - startTime / 1000000000.0);
				System.out.println(uniqueStates + " unique states generated");
				System.out.println("--------------------------------------------------");
				return;
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
	}

	
	private ArrayList<Puzzle> generateSuccessors(Puzzle current){
		ArrayList<Puzzle> successors = new ArrayList<>();
	
		boolean validSuccessor = false;

		for(int i = 0; i < 4; i++){
			validSuccessor = current.generateSuccessor(i);	

			if(validSuccessor){
				successors.add()
			}
		}
		

	}


	private void printSolution(){

	}

}
