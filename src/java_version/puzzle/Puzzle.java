package puzzle;

/**
 * @author Jack Robbins jmr226
 * This java source file contains the implementation of the Puzzle of size N in java
 */
public class Puzzle{
	//The size of the row by column dimension
	private int N;
	private short[][] puzzle;

	//The puzzle constructor takes in a number N for the row and column dimensions of the puzzle
	public Puzzle(int N){
		if(N < 3){
		}

		//If it is good set the value
		this.N = N;	
		//Reserve the appropraite space for the puzzle
		puzzle = new short[N][N];
	}



	public void createStartConfig(int complexity){

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
