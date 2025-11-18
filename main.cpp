/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 6 2025
Comments:		Projects III - Coded Messaging System

				Main file

				*** Week 4 ***
				
				What changes were made for this week?

				Functional audio send / receive
				- nComRate was increased to allow for faster audio transmission

				New transmit() and receive() functions
				- code from w5 was integrated into RS232Comm.cpp
				- audio transmission now follows header / payload convention

				Message Information
				- information regarding the message construction state is displayed at the top of the select station
				  menu, including: com port numbers, header setting, encryption type, compression type

				More testing functions
				- compression and encryption functions were integrated into the testing menu, along with pulling random
				  fortunes from FortuneCookies.txt

				  - new

==========================================================================================================================
*/
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Tx.h"
#include "Rx.h"
#include "ui.h"
#include "RS232Comm.h"
#include "sound.h"
#include "audioQueue.h"
#include "config.h"

int	main() {

	//initalization
	initQueue();
	InitializePlayback();
	InitializeRecording();

	//run main loop
	runModeLoop();

	//cleanup
	ClosePlayback();
	CloseRecording();
	clearQueue(); //free all queue memory

	return 0;
}