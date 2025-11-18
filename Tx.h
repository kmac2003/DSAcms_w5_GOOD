/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Transmitting header file

==========================================================================================================================
*/
#pragma once

#include <Windows.h>

//text
void instantTextMsg(HANDLE* hComTx);
void selectTextType(HANDLE* hComTx);
void sendTextWithHeader(HANDLE* hComTx);
void advancedTextMsg();

//audio
void recordAndSendAudio(HANDLE* hComTx);
void newAudioOptions();
void listenToMsg();

//main transmitter loop
void transmitterLoop(HANDLE* hComTx);
