/*

================================== SECTION HEADER =====================================

Program Name:   PROG72365-25F
Author:         KIEN MACARTNEY
Date:           SEPT 29 2025
Comments:		Data Structures & Algorithms Assignment #1: fileIO cpp file

=======================================================================================

*/
#include "fileIO.h"

// Function gets a random number between min and max (max is the number of quotes in the file)
int frandNum(int min, int max) {
	return((rand() % (max - min + 1)) + min);
}

// Function returns number of quotes in the file (only need to run once)
int fnumQuotes(void) {
	FILE* fp;
	errno_t err;
	char c;
	int counter = -1;
	
	if ((err = fopen_s(&fp, "FortuneCookies.txt", "r")) == 0) {

		//read the file one character at a time
		//look for two percent signs, if we find one then look for another. if we find another, increment the counter

		while ((c = fgetc(fp)) != EOF) {
			if (c == '%') {
				if (fgetc(fp) == '%') {
					counter++;
				}
			}
		}

		fclose(fp);
		return(counter);
	}
	else { //protects against any issues opening the file
		printf("\nERROR: Failed to open file\n");
		fclose(fp);
		return(-1);
	}
}

// Function returns an array that indicates the start of every quote in the file (number of characters from the start of the file) 
long int* fquoteIndices(int numQuotes) {
	FILE* fp;
	errno_t err;
	char c;
	int counter = 0;

	//allocate memory for the indicies
	long int* indicies = (long int*)malloc(sizeof(long int) * numQuotes);

	//show an error if malloc fails
	if (indicies == NULL) {
		printf("ERROR: Memory allocation failed\n");
		return NULL;
	}

	if ((err = fopen_s(&fp, "FortuneCookies.txt", "r")) == 0) {
		//read the file one character at a time
		//look for two percent signs; if found, record the position

		while ((c = fgetc(fp)) != EOF && counter < numQuotes) {
			if (c == '%') {
				if (fgetc(fp) == '%') {
					//record the current position just after the %%
					indicies[counter++] = ftell(fp);
				}
			}
		}
		fclose(fp);
		return indicies;
	}
	else { //this protects against any issues opening the file
		printf("\nERROR: Failed to open the file\n");
		return NULL;
	}
}

// Function returns the smaller of the actual quote length or MAX_QUOTE_LENGTH
int* fquoteLength(int numQuotes, long int* quoteIndices) {
	FILE* fp;
	errno_t err;
	char c;
	int counter = 0;

	//allocate memory for lengths
	int* lengths = (int*)malloc(sizeof(int) * numQuotes);

	if (lengths == NULL) {
		printf("ERROR: Memory allocation failed\n");
		return NULL;
	}

	if ((err = fopen_s(&fp, "FortuneCookies.txt", "r")) == 0) {
		//loop through each quote index
		for (int i = 0; i < numQuotes; i++) {
			int len = 0;
			fseek(fp, quoteIndices[i], SEEK_SET);

			//read until %% or EOF
			while ((c = fgetc(fp)) != EOF) {
				if (c == '%') {
					if (fgetc(fp) == '%') {
						break; //end of this quote
					}
				}
				len++;
				if (len >= MAX_QUOTE_LENGTH) {
					break; //don't exceed max quote length
				}
			}
			//store smaller of actual length or MAX_QUOTE_LENGTH
			if (len < MAX_QUOTE_LENGTH) {
				lengths[counter] = len;
			}
			else {
				lengths[counter] = MAX_QUOTE_LENGTH;
			}
			counter++;
		}
		fclose(fp);
		return lengths;
	}
	else {
		printf("\nERROR: Failed to open file\n");
		return NULL;
	}
}

// Function that gets q random quote from the FortuneCookies file 
int GetMessageFromFile(char* buff, int iLen, int randNum, int numQuotes, long int* quoteIndices, int* quoteLengths) {
	FILE* fp;
	errno_t err;
	char c;
	int len = 0;
	int maxRead; //decide how many charcters we can safely read

	//check for invalid inputs
	if (randNum < 0 || randNum >= numQuotes) {
		printf("ERROR: Invalid quote index\n");
		return -1;
	}

	if ((err = fopen_s(&fp, "FortuneCookies.txt", "r")) == 0) { //if the file opens successfully
		//move to the sart of the chosen quote
		fseek(fp, quoteIndices[randNum], SEEK_SET); // ignore the first character UPDATE, MKAE USR EOT REMOVE NEW LINE

		if (quoteLengths[randNum] < iLen - 1) {
			maxRead = quoteLengths[randNum];
		}
		else {
			maxRead = iLen - 1; //leave space for null
		}

		//read charcters until the end of the quote or buffer is full
		c = fgetc(fp); // skip first character
		while (len < maxRead && (c = fgetc(fp)) != EOF) {
			if (c == '%') {
				if (fgetc(fp) == '%') {
					break; //reached the end of the quote
				}
				else {
					//not actually a double percent, rewind one char
					fseek(fp, -1, SEEK_CUR);
				}
			}
			buff[len] = c; 
			len++;
		}
		buff[len-1] = '\0'; //null terminate the string // UPDATE ADDED -1 TO REMOVE EXTRA NEW LINE
		fclose(fp);
		return 0;
	}
	else { //protects against any issues opening the file
		printf("ERROR: Failed to open file\n");
		return -1;
	}
}

//function to display the quote
void displayQuote(int result, char* testBuff) {
	if (result == 0) {
		printf("\nYour fortune is...\n");
		printf("%s\n", testBuff);
	}
}

void getRandQuote(char* quote){
	int numQuotes;					// Number of quotes in the file
	long int* quoteIndices;			// Array of quote locations in the file (index correspondes to quote number)
	int* quoteLengths;				// Array of quote lengths (index correspondes to quote number)
	int result;						 // result = 0 if successfully get a message

	// Seed the random number generator
	numQuotes = fnumQuotes();								// Number of quotes
	quoteIndices = fquoteIndices(numQuotes);					// Index locations of the quotes
	quoteLengths = fquoteLength(numQuotes, quoteIndices);		// Length of each quote (up to MAX_QUOTE_LENGTH) - cut off after 	

	// Get the random message from the file
	result = GetMessageFromFile(quote, MAX_QUOTE_LENGTH, frandNum(0, numQuotes), numQuotes, quoteIndices, quoteLengths);  // Later replace testBuff with 'node->msg.buff' which is a member of a node struct 

	if (result != 0) {
		printf("ERROR READING FOR QUOTE\n\n");
		quote = NULL;
	}
	printf("\nFortune: %s\n", quote);
}
