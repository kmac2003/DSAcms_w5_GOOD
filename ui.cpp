/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

                User interface implementation file

==========================================================================================================================
*/
#define _CRT_SECURE_NO_WARNINGS
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
#include "settings.h"
#include "testing.h"

int running = TRUE;

//get current time or date
struct tm getTimeStruct() {
    time_t t = time(NULL);
    struct tm* current_time = localtime(&t);
    return *current_time;
}

//w1
//record new audio and save it in the front of the queue
void recordNew() {
    system("cls");
    char name[MAX_FILENAME];
    printf("\nEnter a name for this recording: ");
    scanf_s("%63s", name, (unsigned)_countof(name));
    while (getchar() != '\n'); //flush input

    RecordBuffer(iBigBuf, lBigBufSize);
    CloseRecording();

    printf("\nRecording complete!\n");
    printf("Adding message '%s' to queue...\n", name);
    enqueue(iBigBuf, lBigBufSize, name);

    //reopen recording device for the next recording
    InitializeRecording();
}

//save the front message in the queue to file
void saveFront(){
    if (isQueueEmpty()) {
        printf("\nQueue is empty. Nothing to save\n");
        return;
    }
    link frontNode = peekQueue();
    saveAudio(frontNode->Data.buffer, frontNode->Data.size, frontNode->Data.filename);
}

//play the front message in the queue
void playFront(){
    if (isQueueEmpty()) {
        printf("\nQueue is empty. No messages to play\n");
        return;
    }
    link frontNode = peekQueue();
    printf("\nPlaying message: %s\n", frontNode->Data.filename);
    PlayBuffer(frontNode->Data.buffer, frontNode->Data.size);
    ClosePlayback();
    InitializePlayback();
}

//delete the front message in the queue
void deleteFront(){
    if (isQueueEmpty()) {
        printf("\nQueue is empty. Nothing to delete\n");
        return;
    }
    link deleted = deQueue();
    printf("Deleted message: %s\n", deleted->Data.filename);

    //free audio buffer
    free(deleted->Data.buffer);
    free(deleted);
}

//**********************************************************************************    MENU QOL FUNCTIONS
//program quit
void quit() {
    printf("\nQuitting program...\n");
    Sleep(500);
    exit(0);
}

//notify user of invalid input
void invalid() {
    printf("\nINVALID SELECTION\n");
    Sleep(1000);
    system("cls");
}

//notify user, delay and clear screen
void goBack() {
    printf("\nReturning...\n");
    Sleep(500);
    system("cls");
}

//delay and clear screen
void clearScreen(){
    Sleep(1000);
    system("cls");
}

//collect choice from user
int getInput() {
    int value;
    printf("Select an option: ");
    scanf_s("%d", &value);
    while (getchar() != '\n'); // flush the extra newline from input buffer
    return value;
}

//w2
//**********************************************************************************    PRINTF MENUS
//select whether the program functions as a transmitter or receiver
int selectStation() {
    struct tm now = getTimeStruct();
    printf("================================\n");
    printf("    CMS: Kien Matthew Troy\n");
    printf("    Date: (%02d:%02d:%02d)\n", now.tm_mday, (now.tm_mon + 1), (now.tm_year) + 1900);
    printf("================================\n");
    printf("1 - Transmit\n");
    printf("2 - Receive\n");
    printf("3 - Testing\n");
    printf("4 - Phonebook\n");
    printf("5 - Settings\n");
    printf("6 - Quit\n\n");
    int mode = getInput();
    return mode;
}

//**********************************************************************************    RECEIVER MENUS
//selects what kind of message user wishes to receive
void receivingMenu(){
    printf("\n============= RECEIVING STATION ================\n");
    printf("1. Play recent text message\n");
    printf("2. Play recent audio message\n");
    printf("3. Back");
    printf("\n===========================================================\n");
}

//**********************************************************************************    TRANSMITTER MENUS
//selects what kind of message user wishes to send
void transmittingMenu() {
    printf("\n============= TRANSMITTING STATION ================\n");
    printf("1. Instant text\n");
    printf("2. Compose header\n");
    printf("3. Back");
    printf("\n===========================================================\n");
}

//**********************************************************************************    SETTINGS MENU
//diplays setting choices
void settingsMenu() {
    printf("\n============= SETTINGS ================\n");
    printf("1. Set COM port numbers\n");
    printf("2. Toggle headers\n");
    printf("3. Set encryption type\n");
    printf("4. Set compression type\n");
    printf("5. Set SID\n");
    printf("6. Back");
    printf("\n===========================================\n\n");
}

//**********************************************************************************    TESTING MENU
//displays testing cases
void testingMenu() {
    printf("\n============= TESTING ================\n");
    printf("1. ** Loopback\n");
    printf("2. ** Construct header\n");
    printf("3. ** Sorting queue\n");
    printf("4. ** Error detection\n");
    printf("5. Encrypt / decrypt\n");
    printf("6. Compress / decompress\n");
    printf("7. Back");
    printf("\n===========================================================\n");
}

//**********************************************************************************    CORE LOOP
//runs the core Tx/Rx system
void runModeLoop(){
    srand(time(NULL));
    
    wchar_t txPortName[10];
    wchar_t rxPortName[10];

    while (running) {

        //display program conditions
        loadConfig(&txPortNum, &rxPortNum, &setHeader, &setEncrypt, &setCompress, &senderID);
        
        swprintf(txPortName, 10, L"COM%d", txPortNum); //formats PortNum into L"COM#"
        swprintf(rxPortName, 10, L"COM%d", rxPortNum);

        int mode = selectStation(); //users decide if this is transmitter or receiver

        switch (mode) {
        case TRANSMITTER:
            hComTx = setupComPort(txPortName, nComRate, nComBits, timeout); //setup Tx port
            transmitterLoop(&hComTx); //run transmitter menu loop
            CloseHandle(hComTx);
            purgePort(&hComTx);
            break;

        case RECEIVER:
            hComRx = setupComPort(rxPortName, nComRate, nComBits, timeout); //setup Rx port
            receiverLoop(&hComRx); //run receiver menu loop
            CloseHandle(hComRx);
            purgePort(&hComRx);
            break;

        case TESTING:
            testingLoop();
            break;

        case PHONEBOOK:
            system("cls");
            printf("phonebook\n");
            break;

        case SETTINGS:
            settingsLoop();
            break;

        case QUIT:
            quit();
            break;

        default: //invalid
            invalid();
            continue;
        }
    }
}

