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
	//Use a 2d array for the puzzle
	private short[][] puzzle;
	//Keep track of the zero row and zero column
	private short zeroRow, zeroColumn;
	/* Heuristic function values */
	private int currentTravel;
	private int heuristicValue;
	private int predictionValue;
	/* ========================= */
	//Keep track of the predecessor
	private Puzzle predecessor;


	//The puzzle constructor takes in a number N for the row and column dimensions of the puzzle
	public Puzzle(int N){
		this.N = N;	
		//Reserve the appropraite space for the puzzle
		puzzle = new short[N][N];
		//Call the helper to create the goal config
		this.createGoalConfig();
		//Important: set the predecessor to be null for solution traceback end
		this.predecessor = null;
	}


	//The copy constructor takes in a puzzle of an identical size and performs a deep copy
	public Puzzle(Puzzle predecessor){
		this.N = predecessor.N;
		this.puzzle = new short[N][N];

		//Perform a deep copy of the puzzle array
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				this.puzzle[i][j] = predecessor.puzzle[i][j];
			}
		}

		//Copy over the zero row and column
		this.zeroRow = predecessor.zeroRow;
		this.zeroColumn = predecessor.zeroColumn;

		//The current travel is now one more
		this.currentTravel = predecessor.currentTravel + 1;

		//Set the predecessor to be the copied puzzle
		this.predecessor = predecessor;
	}


	/**
	 * Turn the calling object into a starting config by randomly moving the 0 tile around
	 * for a given number of times
	 */
	public void createStartConfig(int maxMoves){
		Random rand = new Random(1000000000);
		//Get a random integer for our random move
		int direction; 
	
		//Randomly move the puzzle around for complexity moves
		for(int move = 0; move < maxMoves; move++){
			//Get the next random move value
			direction = rand.nextInt() % 4;

			//0 = left move
			if(direction == 0 && this.zeroColumn > 0){
				this.moveLeft();
			}

			//1 = right move
			if(direction == 1 && this.zeroColumn < N - 1){
				this.moveRight();
			}
			
			//2 = down move
			if(direction == 2 && this.zeroRow < N - 1){
				this.moveDown();
			}

			//3 = up move
			if(direction == 3 && this.zeroRow > 0){
				this.moveUp();
			}
		}
	}



	/**
	 * A simple private helper method that mathematically generates the start config for any given N
	 */
	private void createGoalConfig(){
		short row, col;
		//Go through each tile, mathematically finding position
		for(short tile = 1; tile < this.N * this.N; tile++){
			row = (short)((tile - 1) / this.N);	
			col = (short)((tile - 1) % this.N);
			//Store the tile value in the proper position
			this.puzzle[row][col] = tile;
		}
		//Zero row and column always start out as the last
		this.zeroRow = this.zeroColumn = (short)(N-1);
		//Save in the zero slider
		this.puzzle[zeroRow][zeroColumn] = 0;
	}

	
	/**
	 * A simple helper function that swaps two given tiles
	 *
	 * Precondition: all row and column positions have been checked and are valid
	 */
	private void swap(int row1, int column1, int row2, int column2){
		short tile = this.puzzle[row1][column1];
		this.puzzle[row1][column1] = this.puzzle[row2][column2];
		this.puzzle[row2][column2] = tile;
	}


	/**
	 * A helper function that swaps the zero slider with its leftmost neighbor
	 */
	private void moveLeft(){
		swap(this.zeroRow, this.zeroColumn, this.zeroRow, this.zeroColumn - 1);
		//Keep position up to date
		this.zeroColumn--;
	}


	/**
	 * A helper function that swaps the zero slider with its rightmost neighbor
	 */
	private void moveRight(){
		swap(this.zeroRow, this.zeroColumn, this.zeroRow, this.zeroColumn + 1);
		//Keep position up to date
		this.zeroColumn++;
	}


	/**
	 * A helper function that swaps the zero slider with its upper neighbor
	 */
	private void moveUp(){
		swap(this.zeroRow, this.zeroColumn, this.zeroRow - 1, this.zeroColumn);
		//Keep position up to date
		this.zeroRow--;
	}


	/**
	 * A helper function that swaps the zero slider with its lower neighbor
	 */
	private void moveDown(){
		swap(this.zeroRow, this.zeroColumn, this.zeroRow + 1, this.zeroColumn);
		//Keep position up to date
		this.zeroRow++;
	}


	/**
	 * An equals method for comparing puzzles 
	 */
	@Override
	public boolean equals(Object other){
		//Check if we have the same class
		if(other.getClass() != Puzzle.class){
			return false;
		}
		
		//Cast to puzzle
		Puzzle o = (Puzzle)other;

		//Efficiency speedup -- compare zero positions
		if(this.zeroRow != o.zeroRow || this.zeroColumn != o.zeroColumn){
			return false;
		}

		//If that fails, go tile by tile
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				//One mismatch is all we need to return false
				if(this.puzzle[i][j] != o.puzzle[i][j]){
					return false;
				}
			}
		}

		//If we get here they are the same, so
		return true;
	}


	/**
	 * Simple toString() that prints out the puzzle board
	 */
	@Override
	public String toString(){
		String retVal = "";
		//Print out tile by tile
		for(int i = 0; i < N; i++){
			for(int j = 0; j < N; j++){
				//Use string format for spacing
				retVal += String.format("%2d ", this.puzzle[i][j]);
			}
			//Give the effect of rows with newline
			retVal += "\n";
		}
	
		return retVal;
	}
}
