package puzzle;

//For Random puzzle generation
import java.util.Random;


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



	public void createStartConfig(int maxMoves){
		Random rand = new Random(4);
		//Get a random integer for our random move
		int direction = rand.nextInt(0, 3); 
	
		//Randomly move the puzzle around for complexity moves
		for(int move = 0; move < maxMoves; move++)

	}

	private void swap(){

	}

	private void moveLeft(){

	}

	private void moveRight(){

	}

	private void moveUp(){

	}

	private void moveDown(){

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
