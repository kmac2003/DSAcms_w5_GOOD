/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 11 2025
Comments:		Projects III - Coded Messaging System

                Settings implementation file

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
#include "settings.h"

//headers
int setHeader = 0;
int inToggleHeader = TRUE;
//encryption
int setEncrypt = 0;
int inEncryptType = TRUE;
//compression
int setCompress = 0;
int inCompressType = TRUE;
//SID
int senderID = 0;
int inSID = TRUE;

//**********************************************************************************    MODIFY SETTING FUNCTIONS
//allow user to select desired com port numbers
void configureComPorts() {
    system("cls");
    printf("=== COM Port Configuration ===\n");
    printf("Current COM ports: Tx = COM%d, Rx = COM%d\n", txPortNum, rxPortNum);

    printf("\nEnter new Transmitter COM number: ");
    int newTx;
    scanf_s("%d", &newTx);

    printf("Enter new Receiver COM number: ");
    int newRx;
    scanf_s("%d", &newRx);

    Sleep(500);
    system("cls");
    Sleep(500);

    txPortNum = newTx; //assign new com port numbers to original pointers
    rxPortNum = newRx;

    saveConfig(txPortNum, rxPortNum, setHeader, setEncrypt, setCompress, senderID);

    printf("\nCOM ports updated: Tx = COM%d, Rx = COM%d\n", txPortNum, rxPortNum);
}

//allow user to turn message headers on/off
void toggleHeader() {
    system("cls");
    inToggleHeader = TRUE;
    while (inToggleHeader) {
        printf("\n === Enable or disable headers === \n");
        printf("\n1 for ON\n0 for OFF\n\n");

        int choice = getInput();

        if (choice == ON || choice == OFF) { //if headers are turned on or off
            setHeader = choice;
            printf("\nHeaders %s!\n", setHeader ? "ENABLED" : "DISABLED");
            saveConfig(txPortNum, rxPortNum, setHeader, setEncrypt, setCompress, senderID);
            break;
        }
        else {
            invalid();
        }
        clearScreen();
    }
}

//set encryption type (Vigenere or XOR)
void encryptType(){
    system("cls");
    inEncryptType = TRUE;
    while (inEncryptType) {
        printf("\n === Set encryption type === \n");
        printf("\n1 for XOR\n2 for VIGENERE\n0 to disable\n\n");

        int choice = getInput();

        if (choice == XOR || choice == VIGENERE || choice == OFF) { //if encryption is XOR, VIGENERE or OFF
            setEncrypt = choice;
            if (setEncrypt == XOR) {
                printf("\nEncryption set to XOR\n");
            }
            else if (setEncrypt == VIGENERE) {
                printf("\nEncryption set to VIGENERE\n");
            }
            else {
                printf("\nEncryption OFF\n");
            }
            saveConfig(txPortNum, rxPortNum, setHeader, setEncrypt, setCompress, senderID);
            break;
        }
        else {
            invalid();
        }
        clearScreen();
    }
}

//set encryption type (Huffman or RLE)
void compressType(){
    system("cls");
    inCompressType = TRUE;
    while (inCompressType) {
        printf("\n === Set compression type === \n");
        printf("\n1 for HUFFMAN\n2 for RLE\n0 to disable\n\n");

        int choice = getInput();

        if (choice == HUFFMAN || choice == RLE || choice == OFF) { //if compression is HUFFMAN, RLE or OFF
            setCompress = choice;
            if (setCompress == HUFFMAN) {
                printf("\nCompressing with HUFFMAN\n");
            }
            else if (setCompress == RLE) {
                printf("\nCompressing with RLE\n");
            }
            else {
                printf("\nCompression OFF\n");
            }
            saveConfig(txPortNum, rxPortNum, setHeader, setEncrypt, setCompress, senderID);
            break;
        }
        else {
            invalid();
        }
    }
}

//write sender ID number
void configSID(){
    system("cls");
    printf("\n === Configure SID Number === \n");

    int number = getInput();

    if (number >= 0 && number <= 255) {
        senderID = number;
        printf("\nSender ID set to: %d\n", number);
        saveConfig(txPortNum, rxPortNum, setHeader, setEncrypt, setCompress, senderID);
    }
    else {
        invalid();
    }
    clearScreen();
}

//**********************************************************************************    STATE DISPLAY FUNCTIONS
//display whether headers are ON or OFF
void displayHeaderState() {
    if (setHeader == OFF) {
        printf("Headers:\tOFF\n");
    }
    else {
        printf("Headers:\tON\n");
    }
}

//display what kind of encryption is used
void displayEncryptionType(){
    if (setEncrypt == XOR) {
        printf("Encryption:\tXOR\n");
    }
    else if (setEncrypt == VIGENERE){
        printf("Encryption:\tVIGENERE\n");
    }
    else {
        printf("Encryption:\tOFF\n");
    }
}

//display what kind of compression is used
void displayCompressionType(){
    if (setCompress == HUFFMAN) {
        printf("Compression:\tHUFFMAN\n");
    }
    else if (setCompress == RLE){
        printf("Compression:\tRLE\n");
    }
    else {
        printf("Compression:\tOFF\n");
    }
}

//display sender ID
void displaySID() {
    printf("SID:\t\t%d\n", senderID);
}

//display com port numbers
void displayComPorts() {
    printf("Tx:\t\tCOM_%d\n", txPortNum);
    printf("Rx:\t\tCOM_%d\n", rxPortNum);
}

//switch case for all setting configurations
void settingsLoop() {
    system("cls");
    int inSettings = TRUE;

    while (inSettings) {
        settingsMenu();
        int setOption = getInput(); //collect setting choice

        switch (setOption) {
        case CONFIG_COM:
            configureComPorts();
            break;

        case TOGGLE_HEADERS:
            toggleHeader();
            break;

        case ENCRYPT_TYPE:
            encryptType();
            break;

        case COMPRESS_TYPE:
            compressType();
            break;

        case SENDERID:
            configSID();
            break;

        case Set_GO_BACK:
            goBack();
            inSettings = FALSE;
            //return to main menu
            break;

        default:
            invalid();
            break;
        }
    }
}