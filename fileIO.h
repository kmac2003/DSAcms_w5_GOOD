/*

================================== SECTION HEADER =====================================

Program Name:   PROG72365-25F
Author:         KIEN MACARTNEY
Date:           SEPT 29 2025
Comments:		Data Structures & Algorithms Assignment #1: fileIO header file

=======================================================================================

*/
#ifndef FILE_IO_H
#define FILE_IO_H
#define MAX_QUOTE_LENGTH 250

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>			// Dynamic memory allocation for arrays that store quote location and length
#include <errno.h>
#include <time.h>


// These functions will produce the inputs to the GetMessageFromFile() function
int frandNum(int min, int max);							// Function gets a random number between min and max (max is the number of quotes in the file)
int fnumQuotes(void);									// Function returns number of quotes in the file (only need to run once)
long int* fquoteIndices(int numQuotes);					// Function returns an array that indicates the start of every quote in the file (number of characters from the start of the file) 
int* fquoteLength(int numQuotes, long int* quoteIndices); // Function returns the smaller of the actual quote length or MAX_QUOTE_LENGTH


// Function that gets q random quote from the FortuneCookies file 
int GetMessageFromFile(char* buff, int iLen, int randNum, int numQuotes, long int* quoteIndices, int* quoteLengths);  // buff is a character array (need top pass the node element that is a character array) 

//function to display the quote
void displayQuote(int result, char* testBuff);

//get random quote
void getRandQuote(char* quote);

#endif // !FILE_IO_H