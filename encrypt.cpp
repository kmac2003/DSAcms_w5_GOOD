/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 11 2025
Comments:		Projects III - Coded Messaging System

				XOR Encrpytion implementation file

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

//**********************************************************************************    XOR ENCRYPT / DECRPYT
//xor encrpyt
void xorEncrypt(const char* input, char* output, char key) {
	int i = 0;
	while (input[i] != '\0') {
		output[i] = input[i] ^ key;
		i++;
	}
	output[i] = '\0';
}

//xor decrpyt
void xorDecrypt(const char* input, char* output, char key) {
	int i = 0;
	while (input[i] != '\0') {
		output[i] = input[i] ^ key; // same as encode
		i++;
	}
	output[i] = '\0';
}

