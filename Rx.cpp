/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Receiver implementation file

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

//variables
char msgIn[BUFSIZE];
BOOL listening = TRUE;
DWORD bytesRead = 0;

//**********************************************************************************    RECEIVING TEXTS
//continuously receives and displays new text messages until user decides to stop
void playText(HANDLE* hComRx){
	system("cls");
	printf("\nListening for incoming text messages...\n");
	printf("Press 'q' to stop listening\n\n");

	listening = TRUE;
	while (listening) { //check if user wants to stop listening
		if (_kbhit()) { //scan for the letter q, if pressed flag user and return to Rx menu
			char ch = _getch();
			if (ch == 'q' || ch == 'Q') {
				listening = FALSE;
				printf("\nStopped listening\n");
				clearScreen();
				break;
			}
		}
		struct tm now = getTimeStruct(); //get the current time
		//attempt to read from com port
		bytesRead = inputFromPort(hComRx, msgIn, BUFSIZE - 1);
		if (bytesRead > 0) {
			msgIn[bytesRead] = '\0';
			printf("(%02d:%02d:%02d) %s\n", now.tm_hour, now.tm_min, now.tm_sec, msgIn);
		}
		Sleep(100);
	}
}

//**********************************************************************************    RECEIVING AUDIO
void receiveAudioAndPlay(HANDLE* hComRx) {
	Header rxHeader;
	void* rxPayload = NULL;   // this will be allocated inside receive()
	DWORD bytesRead;

	printf("Waiting for audio transmission...\n");

	// Receive header + payload
	bytesRead = receive(&rxHeader, &rxPayload, hComRx);
	if (bytesRead == 0 || rxPayload == NULL) {
		printf("Error: no data received.\n");
		return;
	}

	printf("Received payload: %ld bytes (type '%c')\n", rxHeader.payloadSize, rxHeader.payLoadType);

	// --- CAST BUFFER BACK TO AUDIO FORMAT ---
	short* audioBuffer = (short*)rxPayload;  // convert from void* to short*
	long numSamples = rxHeader.payloadSize / sizeof(short);

	// Play the received audio
	if (!InitializePlayback()) {
		printf("Playback initialization failed.\n");
		free(rxPayload);
		return;
	}

	printf("Playing received audio (%ld samples)...\n", numSamples);
	PlayBuffer(audioBuffer, numSamples);
	ClosePlayback();

	// Free the malloc'd buffer
	free(rxPayload);

	printf("Audio playback complete.\n");
}

//main transmitter branch loop
void receiverLoop(HANDLE* hComRx){
	int running = TRUE;
	while (running) {
		receivingMenu();
		int Rx_choice = getInput();

		switch (Rx_choice) {
		case PLAY_TEXT:
			playText(hComRx); //receive and play text messages
			break;

		case PLAY_AUDIO:
			receiveAudioAndPlay(hComRx);
			break;

		case Rx_GO_BACK:
			goBack();
			running = FALSE;
			break;

		default:
			invalid();
			break;
		}
	}

}
