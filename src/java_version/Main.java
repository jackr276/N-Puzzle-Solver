import puzzle.Puzzle;

public class Main{
	public static void main(String[] args){
		Puzzle p = new Puzzle(4);
		System.out.println(p.toString());
		System.out.println("Generating config");

		p.createStartConfig(200);
		System.out.println(p.toString());
	}
}
