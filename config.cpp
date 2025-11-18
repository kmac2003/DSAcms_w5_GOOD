/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           NOV 6 2025
Comments:		Projects III - Coded Messaging System

				Config implementation file

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

#define CONFIG_FILE "config.txt"

void saveConfig(int tx, int rx, int headers, int encrypt, int compress, int sid) {
	FILE* file;
	fopen_s(&file, CONFIG_FILE, "w");
	if (!file) return;

	fprintf(file, "COM_TX=%d\n", tx);
	fprintf(file, "COM_RX=%d\n", rx);
	fprintf(file, "HEADERS=%d\n", headers);
	fprintf(file, "ENCRYPT=%d\n", encrypt);
	fprintf(file, "COMPRESS=%d\n", compress);
	fprintf(file, "SID=%d\n", sid);

	fclose(file);
}

void loadConfig(int* tx, int* rx, int* headers, int* encrypt, int* compress, int* sid) {
	FILE* file; fopen_s(&file, CONFIG_FILE, "r");
	
	if (file == NULL) {
		printf("\nERROR: No config file found. Using defaults\n");
		return;
	}
	char key[32];
	int value;

	while (fscanf(file, "%31[^=]=%d\n", key, &value) == 2) {
		if (strcmp(key, "COM_TX") == 0) {
			*tx = value;
		}
		else if (strcmp(key, "COM_RX") == 0) {
			*rx = value;
		}
		else if (strcmp(key, "HEADERS") == 0) {
			*headers = value;
		}
		else if (strcmp(key, "ENCRYPT") == 0) {
			*encrypt = value;
		}
		else if (strcmp(key, "COMPRESS") == 0) {
			*compress = value;
		}
		else if (strcmp(key, "SID") == 0) {
			*sid = value;
		}
		// unknown keys are ignored — future-proof
	}
	fclose(file);

	printf("=== Configuration Loaded ===\n");
	printf("Tx\t\tCOM%d\n", *tx);
	printf("Rx\t\tCOM%d\n", *rx);
	printf("Headers\t\t%s\n", (*headers ? "ON" : "OFF"));
	printf("Encryption\t");

	switch (*encrypt) {
	case OFF: 
		printf("OFF\n"); 
		break;
	case XOR: 
		printf("XOR\n"); 
		break;
	case VIGENERE: 
		printf("VIGENERE\n"); 
		break;
	default: 
		printf("Unknown (%d)\n", *encrypt); 
		break;
	}

	printf("Compression\t");
	switch (*compress) {
	case OFF: 
		printf("OFF\n"); 
		break;
	case HUFFMAN: 
		printf("HUFFMAN\n"); 
		break;
	case RLE: 
		printf("RLE\n"); 
		break;
	default: 
		printf("Unknown (%d)\n", *compress); 
		break;
	}

	printf("SID\t\t%d\n\n", *sid);
}