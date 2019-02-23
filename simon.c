/*
* Name: Jed Yeo
* Lab Section: L1T
* Student #: 10697829
* Date: December 1st, 2017
* Email: jedyeo@alumni.ubc.ca
* Option chosen: OPTION 2 (Simon Game)
* Purpose: Implement a simple Simon game utilizing the DAQ simulator and C coding. A sequence of randomly generated
*          colors is shown to the user, which the user must then memorize and be able to replicate it
*          by user input, using the 4 switches in mode 6 of the DAQ simulator. The sequence begins with the first color,
*          and then the user must correctly input that color. The sequence will then show the initial color and the next color,
*          and so on. The game ends when the user correctly inputs 5 colors, or if the user inputs the wrong sequence. In either
*          case, a new game begins.
*/

#define _CRT_SECURE_NO_WARNINGS

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <DAQlib.h>
#include <time.h>

// Defining times for Sleep() to pause for, e.g. flashing LEDs or preparation time.
#define FLASHTIME 500
#define PREPARE 3000

// Defining channels for each LED. Note that some LEDs will not be used.
#define GREENLED 0
#define REDLED 1
#define YELLOWLED 2
#define BLUELED 3

// Defining channels for each button, which corresponds to a certain LED.
#define GREENBUTTON 0
#define REDBUTTON 1
#define YELLOWBUTTON 2
#define BLUEBUTTON 3

// Definining constants for ON and OFF states to write to LEDs.
#define ON 1
#define OFF 0

// Defining TRUE or FALSE return statements for flags and the super loop.
#define TRUE 1
#define FALSE 0

// Defining the upper bound for our given range of LEDs as well as the sequence length for which a game continues on for.
#define UPPER 3
#define SEQUENCELENGTH 5

// Function prototypes.
void runSimon(void);
int randInt(int);
void generateSequence(int, int[]);
int checkGuess(void);
void flashWin(void);
void flashLoss(void);

// Main function which prompts for a setup number (6 for the Simon game) and initializes the DAQ, if possible.
int main(void) {
	int setupNum;

	printf("The DAQ hardware is unavailable for use for this simulation. Enter 6 for the Simon Game Simulator: ");
	scanf("%d", &setupNum);

	// Run the program if initialization was successful.
	if (setupDAQ(setupNum))
		runSimon();
	

	// If not, print an error message.
	else 
		printf("Error: Unable to initialize DAQ. Program will now exit.\n");
	
	printf("\n");
	system("PAUSE");
	return 0;
}

void runSimon(void) {
	int sequence[SEQUENCELENGTH]; // Array that is generated anew each pass through the super loop with a random sequence.
	int level;                    // Designates which level of the sequence the user is currently on.
	int countFlash;               // A count that tells how many times to flash an LED
	int guessForLevel;			  // Which guess a user is on, respective to the current level
	int currentLevel;			  // The current level
	int guess;					  // The current guess
	int correct;				  // Correct flag

	while (continueSuperLoop()) {
		printf("Welcome to the Simon game! Game starts in 3 seconds!\n");

		// Generate a random sequence of LEDs to blink.
		generateSequence(SEQUENCELENGTH, sequence);

		// Use this to check the LED values in the generated sequence array to aid development or debug.
		for (int index = 0; index < SEQUENCELENGTH; index++)
			printf("%d\n", sequence[index]);

		currentLevel = 1; // Each pass through the super loop reset the current level the user is playing to 1 (initial level)
		correct = TRUE;   // Each pass through the super loop set the flag for correct to TRUE in case it was set to FALSE if the user guessed wrong.

		Sleep(PREPARE); // Gives time for the user to prepare.

		printf("Game has begun. Good luck!\n");
		
		// Loops through each level of the game, to a maximum of the defined sequence length.
		for (level = 0; level < SEQUENCELENGTH; level++) {      

			// For the level the player is at, write to the respective LEDs to blink the sequence on the simulator. At each level countFlash is
			// reset to zero so the sequence is displayed from the beginning.
			for (countFlash = 0; countFlash < currentLevel; countFlash++) {
				Sleep(FLASHTIME/2);
				digitalWrite(sequence[countFlash], ON);
				Sleep(FLASHTIME/2);
				digitalWrite(sequence[countFlash], OFF);
			}

			// Now that the user has been shown the sequence so far, we must prompt for an input guess. The number of guesses the player
			// must make is equal to the current level the player is on. This loop ensures that the buttons pressed aligns with the LED
			// values within the sequence array.
			for (guessForLevel = 0; guessForLevel < currentLevel; guessForLevel++) { 

				// For each pass through the guess checking loop, we check if at any point the user made a wrong guess. If so,
				// we break the loop and exit out with correct = FALSE and a new game begins.
				if (!correct)
					break;
				
				// Makes a call to the checkGuess function to see which button was pressed by the user.
				guess = checkGuess();

				// If the guess made by the user was wrong for any point in the sequence, we set the correct flag to FALSE and flash the red LED
				// three times to signify such. 
				if (guess != sequence[guessForLevel]) {
					correct = FALSE;
					printf("You lose! Starting new game...\n");
					flashLoss();
				}
			}

			// Continues the breaking from the guess checking loop.
			// Also accounts for the situation where the user got the last guess wrong using an additional boolean statement here
			// with the same premise as the one within the guess checking loop.
			if (!correct)
				break;

			// After finishing a level we must increment the value of currentLevel as the user is moving on from
			// the current level to the next one.
			currentLevel++;
		}

		// If the user made no incorrect guesses then the correct flag remains TRUE throughout the entire super loop and prints
		// a win message and flashes the green LED 3 times to signify a win.
		if (correct == TRUE) {
			printf("You win! Starting new game...\n");
			flashWin();
		}
	}
}

/*
* Function: randInt
* Return: randomly generated integer between the bounds of 0 and 3, inclusive.
* Param: int upper - Upper bound of the provided channels on the DAQ simulator. Provides a range to generate a random positive integer value
*/
int randInt(int upper) { // No need to check for lower bound as rand() will always return an integer that is greater than or equal to zero.
	int randomInt;

	randomInt = rand() % (upper + 1); // Returns random integer with a value from 0, 1, 2, 3

	return randomInt;
}

/*
* Function: generateSequence
* Return: void
* Param: int length - The length of the given array, in this case, the defined SEQUENCELENGTH (5 for this game).
* Param: int sequence[] - Array that will hold the 5 randomly generated numbers which will determine which LEDs are flashed.
*/
void generateSequence(int length, int sequence[]) {
	int index;

	srand((unsigned)time(NULL));

	for (index = 0; index < length; index++) {
		sequence[index] = randInt(UPPER);
	}
}
/*
* Function: checkGuess
* Return: integer value that corresponds to the button channel inputted.
* Param: void
*/
int checkGuess(void) {
	int greenButtonStatus = digitalRead(GREENBUTTON);
	int redButtonStatus = digitalRead(REDBUTTON);
	int yellowButtonStatus = digitalRead(YELLOWBUTTON);
	int blueButtonStatus = digitalRead(BLUEBUTTON);

	// The return value with signifies which button was pressed.
	int guess;

	// Flag to determine when the user presses a button.
	int guessing = TRUE;

	while (greenButtonStatus == OFF && redButtonStatus == OFF && yellowButtonStatus == OFF && blueButtonStatus == OFF) {
		greenButtonStatus = digitalRead(GREENBUTTON);
		redButtonStatus = digitalRead(REDBUTTON);
		yellowButtonStatus = digitalRead(YELLOWBUTTON);
		blueButtonStatus = digitalRead(BLUEBUTTON);
	}

	if (greenButtonStatus == ON) {
			guessing = FALSE;
			guess = GREENBUTTON;
	}

	else if (redButtonStatus == ON) {
			guessing = FALSE;
			guess = REDBUTTON;
	}
	
	else if (yellowButtonStatus == ON) {
			guessing = FALSE;
			guess = YELLOWBUTTON;
	}

	else if (blueButtonStatus == ON) {
			guessing = FALSE;
			guess = BLUEBUTTON;
	}
	
	while (greenButtonStatus == ON || redButtonStatus == ON || yellowButtonStatus == ON || blueButtonStatus == ON)
		if (!continueSuperLoop())
			break;
	
	return guess;	
}

/*
* Function: flashWin
* Return: void, instead just flashes the green LED 3 times to signify a win
* 
*/
void flashWin(void) {
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, OFF);
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, OFF);
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(GREENLED, OFF);
}

/*
* Function: flashLoss
* Return: void, instead just flashes the red LED 3 times to signify a loss
*/
void flashLoss(void) {
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, OFF);
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, OFF);
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, ON);
	Sleep(FLASHTIME / 3);
	digitalWrite(REDLED, OFF);
}