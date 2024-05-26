package puzzle; 


/**
 * @author Jack Robbins
 * This class contains the custom exception for all puzzle issues
 */
public class PuzzleException extends Exception{

	private String message;

	/**
	 * When constructing this error we simply copy the message
	 */
	public PuzzleException(String message){
		this.message = message;	
	}	


	/**
	 * The toString() simply returns our message
	 */
	@Override
	public String toString(){
		return this.message;
	}

}
