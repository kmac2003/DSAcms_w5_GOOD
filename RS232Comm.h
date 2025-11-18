/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				RS232 cable header file

==========================================================================================================================
*/
#pragma once

#define EX_FATAL 1
#define BUFSIZE 140

//w5
typedef struct header Header;

//w5
struct header {
	short int sid;
	short int rid;
	char priority;
	long int payloadSize;		//Number of bytes in payload after this header
	char payLoadType;			//T: Text, A: Audio
	int encryption;				//0: OFF, 1: XOR, 2: Vigenere
	int compression;			//0: OFF, 1: RLE, 2: Huffman
};

extern int nComRate;			// Baud (Bit) rate in bits/second 
extern int nComBits;			// Number of bits per frame
extern COMMTIMEOUTS timeout;	// A commtimeout struct variable

extern int txPortNum;			//holds tx port int
extern int rxPortNum;		    //holds rx port int
extern HANDLE hComRx;			//pointer to receiver com port
extern HANDLE hComTx;			//pointer to transmitter com port

//w5 additions
void transmit(Header* txHeader, void* txPayload, HANDLE* hCom);
DWORD receive(Header* rxHeader, void** rxPayload, HANDLE* hCom);

// Prototype the functions to be used
void initPort(HANDLE* hCom, wchar_t* COMPORT, int nComRate, int nComBits, COMMTIMEOUTS timeout);
void purgePort(HANDLE* hCom);
void outputToPort(HANDLE* hCom, LPCVOID buf, DWORD szBuf);
DWORD inputFromPort(HANDLE* hCom, LPVOID buf, DWORD szBuf);

// Sub functions
void createPortFile(HANDLE* hCom, wchar_t* COMPORT);
static int SetComParms(HANDLE* hCom, int nComRate, int nComBits, COMMTIMEOUTS timeout);

//w2
HANDLE setupComPort(const wchar_t* portName, int nComRate, int nComBits, COMMTIMEOUTS timeout);

//w5
Header buildHeader(long payloadSize, char payloadType);
