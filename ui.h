/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				User interface header file

==========================================================================================================================
*/
#pragma once
#include <Windows.h>
#include <time.h>

//main menu
enum SELECT_MODE {
	TRANSMITTER = 1,
	RECEIVER = 2,
	TESTING = 3,
	PHONEBOOK = 4,
	SETTINGS = 5,
	QUIT = 6
};

//transmitter
enum TRANSMITTER_CHOICES {
	INSTANT_TEXT = 1,
	COMPOSE_HEADER = 2,
	Tx_GO_BACK = 3
};

//receiver
enum RECEIVER_CHOICES {
	PLAY_TEXT = 1,
	PLAY_AUDIO = 2,
	Rx_GO_BACK = 3
};

//settings
enum SETTINGS {
	CONFIG_COM = 1,
	TOGGLE_HEADERS = 2,
	ENCRYPT_TYPE = 3,
	COMPRESS_TYPE = 4,
	SENDERID = 5,
	Set_GO_BACK = 6
};

//testing
enum TESTING {
	LOOPBACK = 1,
	CONSTRUCT_HEADER = 2,
	SORTING_QUEUE = 3,
	ERROR_DETECT = 4,
	ENCRYPT_DECRPYT = 5,
	COMPRESS_DECOMPRESS = 6,
	BACK_TESTING = 7
};

//get current time
struct tm getTimeStruct();

//w1
void recordNew();
void saveFront();
void playFront();
void deleteFront();
void quit();
void invalid();
void goBack();
void clearScreen();
int getInput();

//w2
//main menu
int selectStation();
//receiver
void receivingMenu();
//transmitter
void transmittingMenu();
//settings
void settingsMenu();
//testing
void testingMenu();
//main loop
void runModeLoop();