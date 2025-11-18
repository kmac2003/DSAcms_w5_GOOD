/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 11 2025
Comments:		Projects III - Coded Messaging System

				Settings header file

==========================================================================================================================
*/
#pragma once

#define OFF 0
#define ON 1
#define XOR 1
#define VIGENERE 2
#define HUFFMAN 1
#define RLE 2

extern int setHeader;
extern int setEncrypt;
extern int setCompress;
extern int senderID;

void configureComPorts();
void toggleHeader();
void encryptType();
void compressType();
void configSID();
//display data functions
void displayHeaderState();
void displayEncryptionType();
void displayCompressionType();
void displaySID();
void displayComPorts();
//main settings loop
void settingsLoop();
