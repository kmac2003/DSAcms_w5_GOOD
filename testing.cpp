/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 11 2025
Comments:		Projects III - Coded Messaging System

				Testing implementation file

==========================================================================================================================
*/
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "Tx.h"
#include "Rx.h"
#include "ui.h"
#include "RS232Comm.h"
#include "sound.h"
#include "audioQueue.h"
#include "config.h"
#include "compress.h"
#include "encrypt.h"
#include "settings.h"
#include "testing.h"
#include "fileIO.h"

char currQuote[MAX_QUOTE_LENGTH]; // Buffer to write the message to

//demonstrates huffman compression / decompression
void huffmanDemo() {
    unsigned char input[1024];
    unsigned char compressed[2048];  // must be larger than input
    unsigned char decompressed[1024];
    int compressed_size;
    char choice;

    //get user input
    printf("Enter a string to compress:\n> ");
    fgets((char*)input, sizeof(input), stdin);

    //remove newline character if present
    size_t len = strlen((char*)input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
        len--;
    }

    if (len == 0) {
        printf("Empty input — nothing to compress.\n");
        return;
    }

    //compress using Huffman_Compress
    compressed_size = Huffman_Compress(input, compressed, len);
    printf("\nCompressed %d bytes -> %d bytes.\n", (int)len, compressed_size);

    //display compressed data in hex
    printf("Compressed data (hex):\n");
    for (int i = 0; i < compressed_size; ++i)
        printf("%02X ", compressed[i]);
    printf("\n");

    //ask user if they want to decompress
    printf("\nDecompress the data? (y/n): ");
    choice = getchar();
    if (choice != 'y' && choice != 'Y') {
        printf("Exiting without decompression.\n");
        return;
    }

    //decompress back to text
    Huffman_Uncompress(compressed, decompressed, compressed_size, len);
    decompressed[len] = '\0';  // null-terminate restored string

    //display result
    printf("\nDecompressed string:\n%s\n", decompressed);
}

//strip enter
void stripNewline(char* s) {
	size_t n = strlen(s);
	if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
}

// test function
void xorDemo() {
	char input[256];
	char encoded[256];
	char decoded[256];
	char choice[8];
	char key;

	printf("Enter a character key for XOR encryption: ");
	scanf(" %c", &key);
	getchar(); // consume leftover newline

	printf("\nEnter text to encrypt: ");
	if (!fgets(input, sizeof(input), stdin)) {
		printf("Input error.\n");
		return;
	}
	stripNewline(input);

	xorEncrypt(input, encoded, key);

	printf("\nEncrypted text (raw XOR bytes):\n");
	for (size_t i = 0; i < strlen(encoded); i++) {
		printf("%02X ", (unsigned char)encoded[i]);
	}
	printf("\n");

	printf("\nDecrpyt? (y/n): ");
    if (!fgets(choice, sizeof(choice), stdin)) {
        return;
    }

	if (tolower(choice[0]) == 'y') {
		xorDecrypt(encoded, decoded, key);
		printf("\nDecrypted text:\n%s\n", decoded);
	}
    Sleep(10000);
	clearScreen();
}

//switch case for all testing functions
void testingLoop() {
    system("cls");
    int inTesting = TRUE;
    while (inTesting) {

        testingMenu();
        int testOption = getInput();

        switch (testOption) {
        case LOOPBACK:
            getRandQuote(currQuote);
            break;

        case CONSTRUCT_HEADER:
            break;

        case SORTING_QUEUE:
            break;

        case ERROR_DETECT:
            break;

        case ENCRYPT_DECRPYT:
            xorDemo();
            break;

        case COMPRESS_DECOMPRESS:
            huffmanDemo();
            break;

        case BACK_TESTING:
            goBack();
            inTesting = FALSE;
            break;

        default:
            invalid();
            break;
        }
    }
}
