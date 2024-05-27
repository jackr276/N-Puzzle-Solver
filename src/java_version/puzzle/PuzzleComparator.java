package puzzle;

import java.util.Comparator;

/**
 * @author Jack Robbins
 * The comparator for puzzles for use in the PriorityQueue. Compares based on
 * prediction function value
 */
public class PuzzleComparator implements Comparator<Puzzle>{
	//Define custom comparison logic for puzzles, we only compare by prediction function
	@Override
	public int compare(Puzzle p1, Puzzle p2){
		return p1.getComparisonFunction() - p2.getComparisonFunction();	
	}
}
