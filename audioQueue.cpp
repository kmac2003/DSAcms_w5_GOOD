/*
===================================================== SECTION HEADER =====================================================

Program Name:   EECE72405-25F
Author:			KIEN MACARTNEY
Date:           OCT 30 2025
Comments:		Projects III - Coded Messaging System

				Audio queue implementation file

==========================================================================================================================
*/
#include "audioQueue.h"

link front = NULL;
link rear = NULL;
int messageCount = 0;

//inits an empty queue
void initQueue(){
	front = rear = NULL;
}

//checks if the queue is empty
int isQueueEmpty(){
	return (front == NULL);
}

//adds a new message to the queue
void enqueue(short* buf, long size, const char* name){
	link newNode = (link)malloc(sizeof(Node));
	if (newNode == NULL) {
		printf("ERROR: malloc failed for queuing new node\n"); //check if malloc worked
		return;
	}

	//copy message data into a new buffer
	newNode->Data.buffer = (short*)malloc(size * sizeof(short));
	if (newNode->Data.buffer == NULL) {
		printf("ERROR: malloc failed for message buffer\n");
		free(newNode);
		return;
	}

	memcpy(newNode->Data.buffer, buf, size * sizeof(short)); //make this queue node hold its own copy of the recorded sound
	newNode->Data.size = size;	
	strncpy_s(newNode->Data.filename, MAX_FILENAME, name, _TRUNCATE); //store the label of this recording inside this queue node
	newNode->pNext = NULL; //mark this node as the last in the queue for now

	//link into the queue
	if (isQueueEmpty()) {
		front = rear = newNode;
	}
	else {
		rear->pNext = newNode;
		rear = newNode;
	}
	messageCount++;
	printf("Message enqueued. Total messages: %d\n", messageCount);
}

//removes the first message node from the queue
link deQueue(){
	if (isQueueEmpty()) {
		printf("The queue is empty\n");
		return NULL;
	}
	
	link temp = front;
	front = front->pNext;
	if (front == NULL) {
		rear = NULL;
	}

	messageCount--;
	printf("Dequeued message. Remaining messages: %d\n", messageCount);
	return temp;
}

//frees all queued messages
void clearQueue(){
	while (!isQueueEmpty()) {
		link temp = deQueue();
		if (temp) {
			free(temp->Data.buffer);
			free(temp);
		}
	}
}

//returns a pointer to front node without removing
link peekQueue() {
	if (isQueueEmpty()) {
		printf("Queue is empty\n");
		return NULL;
	}
	return front;
}
