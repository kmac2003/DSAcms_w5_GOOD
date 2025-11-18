/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Audio queue implementation file

==========================================================================================================================
*/
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGES 10 //max number of queued messages, 10 for now
#define MAX_FILENAME 64 //maximum file name size

typedef struct item Item;
typedef struct node Node;
typedef Node* link;

struct item {
	short* buffer;					//pointer to the audio samples
	long size;						//number of samples in the buffer
	char filename[MAX_FILENAME];	//stores file name
};

struct node {
	Item Data;						//holds message labels
	link pNext;						//pointer to the next node
};

//global vars
extern link front;
extern link rear;
extern int messageCount;

//function definitions
void initQueue();
int isQueueEmpty();
void enqueue(short* buf, long size, const char* name);
link deQueue();
void clearQueue();
link peekQueue();
