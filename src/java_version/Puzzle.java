/**
 * @author Jack Robbins jmr226
 * This java source file contains the implementation of the Puzzle of size N in java
 */
public class Puzzle{
	//The size of the row by column dimension
	private int N;
	private short[][] puzzle;

	//The puzzle 
	public Puzzle(int N){
		this.N = N;	
	}




	@Override
	public String toString(){
		String retVal = "";

		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				retVal += this.puzzle[i][j] + " ";
			}
			retVal += "\n";
		}
	
		return retVal;
	}

}
