/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				RS232 cable implementation file

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
#include "compress.h"
#include "encrypt.h"
 
int nComRate = 460800;				// Baud (Bit) rate in bits/second 
int nComBits = 8;					// Number of bits per frame
COMMTIMEOUTS timeout = { 0 };		// A commtimeout struct variable
char selection;													// Tx or Rx (can run two instances of this program - double click the exe file)
Header txHeader;												// Header transmitted 
Header rxHeader;												// Header received
void* rxPayload = NULL;											// Received payload (buffer) - void so it can be any data type
int txPortNum;
int rxPortNum;
HANDLE hComRx;			//pointer to receiver com port
HANDLE hComTx;			//pointer to transmitter com port

//transmits header and payload
void transmit(Header* txHeader, void* txPayload, HANDLE* hCom) {
	// Send header
	outputToPort(hCom, txHeader, sizeof(Header));
	// Send payload
	outputToPort(hCom, txPayload, txHeader->payloadSize);
	//delay to ensure transmission completes
	Sleep(500);
	purgePort(hCom); //flush port buffer
	printf("\nTransmission complete.\n");
}

//receives header and payload
DWORD receive(Header* rxHeader, void** rxPayload, HANDLE* hCom) {
	DWORD bytesRead;
	inputFromPort(hCom, rxHeader, sizeof(Header)); //read header first

	// Allocate space for payload
	*rxPayload = malloc(rxHeader->payloadSize);
	if (*rxPayload == NULL) {
		printf("Error: failed to allocate %ld bytes.\n", rxHeader->payloadSize);
		return 0;
	}
	// Read payload
	bytesRead = inputFromPort(hCom, *rxPayload, rxHeader->payloadSize);
	purgePort(hCom); //flush port buffers

	return bytesRead;
}

// Initializes the port and sets the communication parameters
void initPort(HANDLE* hCom, wchar_t* COMPORT, int nComRate, int nComBits, COMMTIMEOUTS timeout) {
	createPortFile(hCom, COMPORT);						// Initializes hCom to point to PORT#
	purgePort(hCom);									// Purges the COM port
	SetComParms(hCom, nComRate, nComBits, timeout);		// Uses the DCB structure to set up the COM port
	purgePort(hCom);
}

// Purge any outstanding requests on the serial port (initialize)
void purgePort(HANDLE* hCom) {
	PurgeComm(*hCom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

//output messages to ports
void outputToPort(HANDLE* hCom, LPCVOID buf, DWORD szBuf) {
	int i=0;
	DWORD NumberofBytesTransmitted;
	LPDWORD lpErrors=0;
	LPCOMSTAT lpStat=0; 

	i = WriteFile(
		*hCom,										// Write handle pointing to COM port
		buf,										// Buffer size
		szBuf,										// Size of buffer
		&NumberofBytesTransmitted,					// Written number of bytes
		NULL
	);
	// Handle the timeout error
	if (i == 0) {
		printf("\nWrite Error: 0x%x\n", GetLastError());
		ClearCommError(hCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.	
	}
	else {
		//printf("\nTRANSMISSION SUCCESS! %ld bytes transmitted\n", NumberofBytesTransmitted);
	}
}

//input messages from ports
DWORD inputFromPort(HANDLE* hCom, LPVOID buf, DWORD szBuf) {
	BOOL readSuccess;
	DWORD NumberofBytesRead;
	LPDWORD lpErrors = 0;
	LPCOMSTAT lpStat = 0;

	readSuccess = ReadFile(
		*hCom,				// Read handle pointing to COM port
		buf,				// Buffer size
		szBuf,  			// Size of buffer - Maximum number of bytes to read
		&NumberofBytesRead,
		NULL
	);
	// Handle the timeout error
	if (!readSuccess) {
		printf("\nRead Error: 0x%x\n", GetLastError());
		ClearCommError(hCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.
	}
	else if (NumberofBytesRead > 0) {
		//printf("\nRECEPTION SUCCESS! %ld bytes read\n", NumberofBytesRead);
	}
	//else: read succeeded but 0 bytes were read, so do nothing
	return(NumberofBytesRead);
}

// Sub functions called by above functions
/**************************************************************************************/
// Set the hCom HANDLE to point to a COM port, initialize for reading and writing, open the port and set securities
void createPortFile(HANDLE* hCom, wchar_t* COMPORT) {
	// Call the CreateFile() function 
	*hCom = CreateFile(
		COMPORT,									// COM port number  --> If COM# is larger than 9 then use the following syntax--> "\\\\.\\COM10"
		GENERIC_READ | GENERIC_WRITE,				// Open for read and write
		NULL,										// No sharing allowed
		NULL,										// No security
		OPEN_EXISTING,								// Opens the existing com port
		FILE_ATTRIBUTE_NORMAL,						// Do not set any file attributes --> Use synchronous operation
		NULL										// No template
	);
	
	if (*hCom == INVALID_HANDLE_VALUE) {
		printf("\nFatal Error 0x%x: Unable to open\n", GetLastError());
	}
	else {
		printf("\nCOM is now open\n");
	}
}

static int SetComParms(HANDLE* hCom, int nComRate, int nComBits, COMMTIMEOUTS timeout) {
	DCB dcb;										// Windows device control block
	// Clear DCB to start out clean, then get current settings
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(*hCom, &dcb))
		return(0);

	// Set our own parameters from Globals
	dcb.BaudRate = nComRate;						// Baud (bit) rate
	dcb.ByteSize = (BYTE)nComBits;					// Number of bits(8)
	dcb.Parity = 0;									// No parity	
	dcb.StopBits = ONESTOPBIT;						// One stop bit
	if (!SetCommState(*hCom, &dcb))
		return(0);

	// Set communication timeouts (SEE COMMTIMEOUTS structure in MSDN) - want a fairly long timeout
	memset((void *)&timeout, 0, sizeof(timeout));
	timeout.ReadIntervalTimeout = 500;				// Maximum time allowed to elapse before arival of next byte in milliseconds. If the interval between the arrival of any two bytes exceeds this amount, the ReadFile operation is completed and buffered data is returned
	timeout.ReadTotalTimeoutMultiplier = 1;			// The multiplier used to calculate the total time-out period for read operations in milliseconds. For each read operation this value is multiplied by the requested number of bytes to be read
	timeout.ReadTotalTimeoutConstant = 5000;		// A constant added to the calculation of the total time-out period. This constant is added to the resulting product of the ReadTotalTimeoutMultiplier and the number of bytes (above).
	SetCommTimeouts(*hCom, &timeout);
	return(1);
}

//w2
//handle setup function, likely will be reworked later to add chanegable port #
HANDLE setupComPort(const wchar_t* portName, int nComRate, int nComBits, COMMTIMEOUTS timeout){
	system("cls");
	HANDLE hCom;
	initPort(&hCom, (wchar_t*)portName, nComRate, nComBits, timeout);
	return hCom;
}

//w5
//header construction
Header buildHeader(long payloadSize, char payloadType){
	Header h;

	printf("\n--- Message Header ---\n");

	printf("\nSender ID: ");
	scanf_s("%hd", &h.sid);

	printf("\nReceiver ID: ");
	scanf_s("%hd", &h.rid);

	printf("\nPriority (1-5): ");
	scanf_s(" %d", &h.priority);

	h.payloadSize = payloadSize;
	h.payLoadType = payloadType;

	printf("\nEncryption (0: None, 1: XOR, 2: Vigenere): ");
	scanf_s("%d", &h.encryption);

	printf("\nCompression (0: None, 1: RLE, 2: Huffman): ");
	scanf_s("%d", &h.compression);

	while (getchar() != '\n'); // flush input

	printf("Header Created!\n\n");

	return h;
}