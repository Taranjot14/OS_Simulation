#ifndef COMMANDS_H
#define COMMANDS_H


#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <string.h>

#define MAX_MESSAGE_LENGTH 40
#define MAX_PROCESSES 100
#define MAX_SEMAPHORES 5



typedef enum 
{
	queued,
	running,
	blocked,
	

}Status;

typedef enum 
{
	nones,
	needs_to_reply, // for process that was sent a message, and needs to receive and reply
	
	

}replyStatus;

typedef enum
{
	none,
	message_in_inbox,
	waiting_for_response,
	receive_Wout_message,
	prints,
}recieveStatus;

typedef struct 
{
	int orgPriority;
	int curPriority;
	Status status;
	int pid;
	char*message;
	
	replyStatus replystatus;
	recieveStatus recieveStatus;


}Process;

typedef struct {

	int val;
	bool created;
	List* processesWaiting;

} Semaphore;


extern List *queue0,*queue1,*queue2;
extern Process* Running;
extern Process*init;
extern bool first_time;


bool Create(int priority);

bool Fork();

bool Kill(int pid);

bool Exit();

bool Quantum();

bool Send(int pid, char * msg);

bool Receive();

bool Reply(int pid,char* msg);

bool newSemaphore(int semaphore, int initial);

bool SemaphoreP(int semaphore);

bool SemaphoreV(int semaphore);

bool Procinfo(int pid);

void Totalinfo();

void Init();

void initSem();

void starvation();

void print_rec();
#endif