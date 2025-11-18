/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Sound header file

==========================================================================================================================
*/
#pragma once
#include <windows.h>					// Contains WAVEFORMATEX structure

// CONSTANTS - SET RECORDING TIME AND RATE
#define	DEFAULT_NSAMPLES	4000	
#define MIN_BUFSIZE			1000
#define RECORD_TIME			6		// seconds to record from mic
#define SAMPLES_SEC			8000	// number of samples per second
#define	NFREQUENCIES		96		// number of frequencies used - leave this alone

//w1
extern short iBigBuf[]; //global recording buffer
extern long lBigBufSize; //total number of samples

// FUNCTION PROTOTYPES
// 
// Playback
int InitializePlayback(void);
int PlayBuffer(short *piBuf, long lSamples);
void ClosePlayback(void);

// Recording
int InitializeRecording(void);
int	RecordBuffer(short *piBuf, long lBufSize);
void CloseRecording(void);

// Support functions for Playback functions (updated 2021)
void SetupFormat(WAVEFORMATEX* wf);			// Used by InitializePlayback()					
int WaitOnHeader(WAVEHDR* wh, char cDit);	// Used by Playbuffer()

//w1: modularization functions
void saveAudio(short* buffer, long size, const char* filename);