import puzzle.Puzzle;
import solver.Solver;

public class Main{
	public static void main(String[] args){
		Puzzle goal = new Puzzle(Integer.parseInt(args[0]));
		System.out.println("Goal Configuration");
		System.out.println(goal.toString());
		System.out.println("Starting Configuration");
		Puzzle start = new Puzzle(Integer.parseInt(args[0]));
		start.createStartConfig(Integer.parseInt(args[1]));
		System.out.println(start.toString());

		Solver s = new Solver();
	

		s.solve(start, goal);
	}
}
