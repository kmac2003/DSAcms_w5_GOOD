/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Transmitting implementation file

==========================================================================================================================
*/
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

#define TEXT 1
#define AUDIO 2


char msgOut[BUFSIZE];
size_t len;
int receiverID;
int priority;
int msgType;
//menu vars
int instantText = TRUE;
int transmitting = TRUE;
int sendingNewText = TRUE;
int makingHeader = TRUE;
int audioMsg = TRUE;



//**********************************************************************************    SENDING TEXTS
//creates and sends new text messages
void instantTextMsg(HANDLE* hComTx) {
	system("cls");
	printf("\nType your messages below...\n");
	printf("Enter 'q' to stop sending\n");

	instantText = TRUE;
	while (instantText) {
		//collect string from the user
		printf("\nEnter message: ");
		fgets(msgOut, sizeof(msgOut), stdin);

		//trim newline if present
		len = strlen(msgOut);
		if (len > 0 && msgOut[len - 1] == '\n') {
			msgOut[len - 1] = '\0';
			len--;
		}
		//check for quit command
		if (_stricmp(msgOut, "q") == 0) { //if user quits return to Tx menu
			instantText = FALSE;
			printf("\nStopped sending\n");
			clearScreen();
			break;
		}
		//send message
		outputToPort(hComTx, msgOut, strlen(msgOut) + 1);
		//printf("Message sent: %s\n", msgOut);
	}
}

//send text messages with a header
void sendTextWithHeader(HANDLE* hComTx) {
	char msgOut[BUFSIZE];
	size_t len;

	printf("\nEnter message to send (q to quit):\n");
	fgets(msgOut, sizeof(msgOut), stdin);

	len = strlen(msgOut);
	if (len > 0 && msgOut[len - 1] == '\n') {
		msgOut[len - 1] = '\0';
		len--;
	}

	if (_stricmp(msgOut, "q") == 0)
		return;

	// Build header automatically based on text payload
	Header txHeader = buildHeader(len + 1, 'T'); // 'T' = Text

	// Send everything using your existing function
	transmit(&txHeader, msgOut, hComTx);

	printf("\nText Message Sent!\n");
}

//create the outgoing header
void composeHeaderLoop() {
	system("cls");
	makingHeader = TRUE;

	while (makingHeader) {
		printf("Enter Receiver ID:\n> ");
		receiverID = getInput();
		printf("\nReceiver ID set to: %d\n\n", receiverID);

		printf("Enter Message Priority (1-5):\n> ");
		priority = getInput();

		if (priority < 1 || priority > 5) {
			printf("\nInvalid priority! Must be 1–5.\n");
			continue;
		}

		printf("\nPriority set to: %d\n\n", priority);

		printf("Message type:\n1 = Text\n2 = Audio\n> ");

		int choice = getInput();

		if (choice == TEXT) {
			printf("\nText message selected.\n");
		}
		else if (choice == AUDIO) {
			printf("\nAudio message selected.\n");
		}
		else {
			invalid();
			continue;
		}

		printf("\nMessage configuration saved!\n");
		Sleep(700);

		break; // exit loop back to Tx menu
	}

	clearScreen();
}

//**********************************************************************************    SENDING AUDIO
//record and send new audio message
void recordAndSendAudio(HANDLE* hComTx) {
	Header txHeader;

	//initialize recording
	if (!InitializeRecording()) {
		printf("Error: Failed to initialize recording device.\n");
		return;
	}
	//record into buffer
	if (!RecordBuffer(iBigBuf, lBigBufSize)) {
		printf("Error: Recording failed.\n");
		CloseRecording();
		return;
	}
	printf("\n\nRecording complete. (%ld samples)\n", lBigBufSize);

	//prepare header
	txHeader.payLoadType = 'A';  // 'A' for audio
	txHeader.payloadSize = lBigBufSize * sizeof(short);

	//send header and payload
	printf("Sending audio clip (%ld bytes)...\n", txHeader.payloadSize);
	transmit(&txHeader, (void*)iBigBuf, hComTx);

	//clean up
	CloseRecording();
	printf("Audio transmission complete.\n");
}

//asks the user if they'd like to listen to the msg they just recorded
void listenToMsg() {
	char ch;
	transmitting = TRUE;
	printf("\nWould you like to listen to your recording? (y / n)\n");
	while (transmitting) {
		if (_kbhit()) {          // Check if a key has been pressed
			ch = _getch();       // Get the character without waiting for Enter
			if (ch == 'y' || ch == 'Y') {
				playFront(); //play message and then go to audio sub menu
				break;
			}
			else if (ch == 'n' || ch == 'N') {
				//user hit no, skip to audio sub menu
				break;
			}
			else {
				printf("\nInvalid key '%c'. Press 'y' or 'n'.\n", ch);
			}
		}
	}
	system("cls");
}

//**********************************************************************************    MAIN TRANSMITTER LOOP
//main transmitter branch loop
void transmitterLoop(HANDLE* hComTx) {
	transmitting = TRUE;
	while (transmitting) {
		transmittingMenu(); //Tx print menu
		int Tx_choice = getInput();

		switch (Tx_choice) {
		case INSTANT_TEXT:
			instantTextMsg(hComTx);
			break;

		case COMPOSE_HEADER:
			composeHeaderLoop();
			break;

		case Tx_GO_BACK:
			goBack();
			transmitting = FALSE;
			break;

		default:
			invalid();
			break;
		}
	}
}