#include "commands.h"

List *queue1, *queue0, *queue2;
List *blockQueue, *blockedALLsem;
bool first_time;
static int pid_valueGenerator = 1;

Process *init;
Process *Running;

// 0 to 4 sem ID
Semaphore sem[5];

void print_rec() // prints when process gets unblocked from using receive but only when it goes on the Running queue
{
	if (Running->recieveStatus == prints)
	{
		printf("The Message ise: %s",Running->message);
		free(Running->message);
		Running->message = NULL;
		Running->recieveStatus = none;
	}
}

void starvation() // solves the starvation problem read the README
{
	// move everything in queue 1 to queue 0
	while(List_count(queue1) > 0)
	{
		List_first(queue1);
		Process* temp = List_remove(queue1);
		temp->curPriority--;
		List_append(queue0,temp);
	}
	while(List_count(queue2) > 0)
	{
		List_first(queue2);
		Process* temp = List_remove(queue2);
		temp->curPriority--;
		List_append(queue1,temp);
	}
}


void printProcess(Process *item) // prints the process
{
	if (item == NULL)
		return;

	char *message = item->message;

	printf("  PID: %d\n  Orignal Priority: %d\n  Current Priority: %d \n", item->pid, item->orgPriority, item->curPriority);

	if (item->recieveStatus == none)
		printf("  message status: None\n");
	else if (item->recieveStatus == waiting_for_response)
		printf("  message status: Waiting and blocked (excluding init its not blocked)\n");
	else if(item->recieveStatus == receive_Wout_message)
	{
		printf("  message status: blocked from recevie that did not have incoming message(excluding init its not blocked)\n");
	}
	else if(item->recieveStatus == prints)
	{
		printf("  Process been unblocked from receive and will print message when it runs\n");
	}
	else
		printf("  message status: Message in the inbox\n");

	if (item->replystatus == nones)
		printf("  message reply status: None\n");
	else if (item->replystatus == needs_to_reply)
		printf("  message reply status: Can reply \n");

	printf("  Message: %s\n", message);

	if (item->status == queued)
		printf("  Status: queued\n");
	else if (item->status == running)
		printf("  Status: running\n");
	else
		printf("  Status: blocked\n");

	printf("\n");
}

void printQueueInfo(List *queue) // prints the entrie queue of processes
{
	Node *current = queue->pFirstNode; // temp for current

	if (current == NULL) // if null print nothing
	{
		printf("  No processes in the queue.\n");
		return;
	}

	while (current != NULL) // prints every process in the queue
	{
		printProcess((Process *)current->pItem);
		current = current->pNext;
	}
}

void Init() // initialize the init process and the 5 queues
{
	Process *temp = (Process *)malloc(sizeof(Process)); // set the init
	temp->pid = 0;
	temp->curPriority = 3;
	temp->orgPriority = 3;
	temp->status = running;
	temp->message = NULL;
	temp->recieveStatus = none;
	temp->replystatus = nones;
	init = temp;
	Running = init;
	if (!first_time) // set the create 
	{
		queue0 = List_create();
		queue1 = List_create();
		queue2 = List_create();
		blockQueue = List_create();
		blockedALLsem = List_create();
		// Set it to to true, for after first time
		first_time = true;
	}
}

void initSem() // set the 5 semaphores
{
	for (int i = 0; i < 5; i++)
	{
		sem[i].val = 0;
		sem[i].processesWaiting = List_create();
		sem[i].created = false;
	}
}

bool compareFunct(void *pItem, void *pComparisonArg) // checks if items match the pid
{
	Process *temp = (Process *)pItem;
	int *pidCompare = (int *)pComparisonArg;
	if (temp->pid == *pidCompare)
		return true;

	return false;
}



// checks if there is a process
bool getfromQueue() // checks if there is at least one process in the queues
{

	if (List_count(queue0) > 0)
	{
		return true;
	}

	else if (List_count(queue1) > 0)
	{
		return true;
	}
	else if (List_count(queue2) > 0)
	{
		return true;
	}
	else if (List_count(blockQueue) > 0)
	{
		return true;
	}
	if (List_count(blockedALLsem) > 0)
		return true;

	return false;
}

Process *getUnblockfromQueue() // gets the first item from the queue, if queue 0 is empty then check the next one
{
	Process *availProcess = NULL;

	if (List_count(queue0) > 0)
	{
		List_first(queue0);
		
			availProcess = List_remove(queue0);
		
	}
	else if (List_count(queue1) > 0)
	{
		List_first(queue1);
		
			availProcess = List_remove(queue1);
		
	}
	else if (List_count(queue2) > 0)
	{
		List_first(queue2);
		
		availProcess = List_remove(queue2);
		
	}

	return availProcess;
}


// Proccess functions ----------------------------------------------------------------------------

bool Create(int priority) // make a process
{
	if (priority < 0 || 2 < priority) // checks the pid
	{
		printf("The value of the priorty provided is to big or small\nit needs to be either 0,1,2\n");
		return false;
	}

	Process *process = (Process *)malloc(sizeof(Process)); // allocate the memory and set data
	process->curPriority = priority;
	process->orgPriority = priority;
	process->status = queued;
	process->pid = pid_valueGenerator++;
	process->message = NULL;
	process->replystatus = nones;
	process->recieveStatus = none;
	if (Running->pid == 0) // check if init is running to replace it with the new process
	{
		process->status = running;
		init->status = queued;
		Running = process;
		return true;
	}
		// else add the process to the queue
	int success;
	if (priority == 0)
		success = List_append(queue0, process); // highest priorty
	else if (priority == 1)
		success = List_append(queue1, process);
	else
		success = List_append(queue2, process);

	if (success == -1)
		return false;
	else
		return true;
}

bool Fork() // makes a duplicate of the process
{
	
	if (Running->pid == 0 || Running == NULL) // if init is running it fails
	{
		printf("CANNOT FORK INIT\n");
		return false;
	}

	Process *process = (Process *)malloc(sizeof(Process)); // makes a duplicate of the process and allocate memory
	process->curPriority = Running->orgPriority;
	process->orgPriority = Running->orgPriority;
	process->status = queued;
	process->pid = pid_valueGenerator++;
	process->message = NULL;
	process->replystatus = nones;
	process->recieveStatus = none;
	int success;
	if (Running->orgPriority == 0)
		success = List_append(queue0, process); // highest priorty
	else if (Running->orgPriority == 1)
		success = List_append(queue1, process);
	else
		success = List_append(queue2, process);

	if (success == -1)
		return false;
	else
		return true;
}

bool Kill(int pid) // kills no matter what excluding init, 
{
	if (Running->pid == pid)
	{
		Exit(); // using function below
		return true;
	}
	if(pid == 0) // cant kill pid when its not runnid
	{
		printf("Cannot kill init while other processes exists!\n");
		return false;
	}
		

	List *queues[] = {queue0, queue1, queue2, blockQueue, blockedALLsem}; // check the process for the process and remove it
	for (int i = 0; i < 5; ++i)
	{
		List_first(queues[i]);
		void *value = List_search(queues[i], compareFunct, &pid);
		if (value != NULL)
		{
			
			Process* temp = (List_remove(queues[i]));
			if (temp->message != NULL)
			{
				free(temp->message);
			}
			free(temp);
			return true;
		}
	}
	printf("PID provided does not exist\n");
	return false;
}

bool Exit() // removes running, not freeing init
{
	if (Running->pid == 0)
	{
		// check if there are blocked processes
		bool exist = getfromQueue();
		// if tempvalue = null then init is the only process remaining and kill simulation
		if (!exist)
		{
			free(Running);
			Running = NULL;
			exit(1); // could pick better way
			return true;
		}
		// if temp does not equal NULL the other processes should be blocked

		return false;
	}

	// this section for if the Running does not have init process, but a normal process
	if (Running->message != NULL)
	{
		free(Running->message);
	}
	free(Running);
	Running = NULL;
	// find a unblock process
	Process *value = getUnblockfromQueue();
	if (value != NULL)
	{
		value->status = running;
		Running = value; // set the new Running
		return true;
	}
	else
	{
		Running = init;
		init->status = running;
		return true;
	}
}

bool Quantum() // simulates a timer running out, so put it back to the queue
{

	if (Running->pid == 0) // init only runs if its the only process
	{
		return false;
	}
	const int priorty = Running->curPriority;
	Process *ChosenOne = getUnblockfromQueue();
	if (ChosenOne == NULL) // if null then the current process gets to stay
	{
		return false;
	}
	Running->status = queued;
	if (priorty == 0) // otherwise add it back to the queue and put the new process to the runnig queue
		List_append(queue0, Running);

	else if (priorty == 1)
		List_append(queue1, Running);
	else if (priorty == 1)
		List_append(queue2, Running);
	ChosenOne->status = running;
	Running = ChosenOne;
	return true;
}

bool Send(int pid, char *msg) {// sends a message to the a process read the readme for more info 

	Process *sendingTO = NULL;
	List *queues[] = {queue0, queue1, queue2, blockQueue, blockedALLsem};
	for (int i = 0; i < 5; i++)
	{
		// look for the process to be sent to
		List_first(queues[i]);
		sendingTO = List_search(queues[i], compareFunct, &pid);
		// found
		if (sendingTO != NULL)
		{
			break;
		}
	}
	if (pid == Running->pid)
	{
		sendingTO = Running;
	}
	if (pid == 0)
	{
		sendingTO = init;
	}
	// if pid not found after all the queues
	if (sendingTO == NULL)
	{
		// print statement if not found then does not block
		printf("PID does not exist in the system\n");
		free(msg);
		return false;
	}
	
	// check if proccess already has a pending message or wrong type of message but stills block and finds a new process
	if (sendingTO->message != NULL || sendingTO->recieveStatus == waiting_for_response)
	{
		
		printf("Process %d already has a pending message or will only recieve from a reply\n", pid);
		free(msg);
		Running->status = blocked;
		Running->recieveStatus = waiting_for_response;
		Running->replystatus = nones;
		List_append(blockQueue, Running); // message blocked queue
		Running = NULL;
		Process *tempvalue = getUnblockfromQueue(); // find new item
		if (tempvalue == NULL)
		{

			Running = init;
			Running->status = running;
			return true;
		}
		Running = (Process *)tempvalue;
		Running->status = running;
		return false;
	}

	
	if (sendingTO->recieveStatus == receive_Wout_message) // if the receiever blocked itself then this will realse it from its block state
	{
		sendingTO->message = msg;
		sendingTO->replystatus = nones;
		sendingTO->recieveStatus = prints;
		List_first(blockQueue);
		List_search(blockQueue, compareFunct, &(sendingTO->pid));
		List_remove(blockQueue); // remove block process from the blocked queue
		if (Running->pid == 0) // replace init
		{
			Running = sendingTO;
			Running->status = running;
			printf("The message is: %s\n",msg);
			free(msg);
			
		}
		else 
		{
			// add back to queue when unblocked
			int priorty = sendingTO->curPriority;
		sendingTO->status = queued;
		if (priorty == 0)
			List_append(queue0, sendingTO);

		else if (priorty == 1)
			List_append(queue1, sendingTO);
		else if (priorty == 1)
			List_append(queue2, sendingTO);
		}
		
			

	}
	else // just a regular send
	{
		sendingTO->message = msg;
	sendingTO->replystatus = needs_to_reply;
	sendingTO->recieveStatus = message_in_inbox;
	}
	
	

	// check if Running is init since it cant get blocked
	if (Running->pid == 0)
	{
		Running->recieveStatus = waiting_for_response;
		Running->replystatus = nones;
		return true;
	}

	Running->status = blocked;
	Running->recieveStatus = waiting_for_response;
	Running->replystatus = nones;
	List_append(blockQueue, Running); // message blocked queue
	Running = NULL;
	Process *tempvalue = getUnblockfromQueue();
	if (tempvalue == NULL)
	{
		
		Running = init;
		Running->status = running;
		return true;
	}
	Running = (Process *)tempvalue;
	Running->status = running;
	return true;
}

bool Receive()
{

	if (Running->recieveStatus == message_in_inbox) // check if it has a message pending
	{
		printf("Message received %s \n", Running->message);
		Running->recieveStatus = none;
		free(Running->message);
		Running->message = NULL;
		return true;
	}
	else // other wise it blocks it self
	{
		if (Running->pid == 0)
		{
			Running->recieveStatus = waiting_for_response;
			return false;
		}
		Running->status = blocked;
		Running->recieveStatus = receive_Wout_message;
		List_append(blockQueue, Running);
		Running = NULL;

		Process *temp = getUnblockfromQueue();
		if (temp == NULL)
		{
			Running = init;
			return false;
		}
		else
		{
			Running = temp;
			return false;
		}
	}
	return true;
}

bool Reply(int pid, char *msg) // if unblocked check if init is running
{
	Process *replyTO = NULL;
	List *queues[] = {queue0, queue1, queue2, blockQueue, blockedALLsem};
	for (int i = 0; i < 5; i++)
	{
		// look for the process to be sent to
		List_first(queues[i]);
		replyTO = List_search(queues[i], compareFunct, &pid);
		// found
		if (replyTO != NULL)
		{
			
			break;
		}
	}
	if (pid == Running->pid) // edge case check
	{
		replyTO = Running;
	}
	if (pid == 0)
	{
		replyTO = init;
	}
	// if pid not found after all the queues
	if (replyTO == NULL)
	{
		free(msg);
		// print statement if not found then does not block
		printf("PID does not exist in the system\n");
		return false;
	}
	if (replyTO->message != NULL) // this case should never occur if implemented correctly
	{
		printf("Process %d already has a pending message\n", pid);
		free(msg);
		return false;
	}

	if (replyTO->recieveStatus == waiting_for_response) // only reply to a process that blocked itself from a send
	{
		List_first(blockQueue);
		List_search(blockQueue, compareFunct, &(replyTO->pid));
		List_remove(blockQueue); // remove block process from the blocked queue
		if (Running->pid == 0)
		{
			Running = replyTO;
			Running->status = running;
			replyTO->message = msg;
			replyTO->recieveStatus = message_in_inbox;
			replyTO->replystatus = nones;
			return true;
		}
		int priorty = replyTO->curPriority;
		replyTO->status = queued;
		if (priorty == 0)
			List_append(queue0, replyTO);

		else if (priorty == 1)
			List_append(queue1, replyTO);
		else if (priorty == 1)
			List_append(queue2, replyTO);
	}
	else if (replyTO->recieveStatus == receive_Wout_message) // if can only get block from send
	{
		free(msg);
		printf("Can only receive from a send\n");
		return false;
	}	
	else
	{
		free(msg);
		printf("The Process that you're replying is currently not waiting for a response\n");
		return false;
	}

	replyTO->message = msg; // give the ex-blocked process its data
	replyTO->recieveStatus = message_in_inbox;
	replyTO->replystatus = nones;
	return true;
}

bool newSemaphore(int semaphore, int initial) // user intialzies the process
{
	// 0 to 4 sem ID
	
	if (semaphore >= 0 && semaphore < 5 && initial >= 0)
	{
		if (!sem[semaphore].created)
		{
			sem[semaphore].val = initial;
			sem[semaphore].created = true;
			return true;
		}
		else
		{
			printf("Sem has already been made!\n");
		}
	}
	else
	{
		printf("Sem ID not found!\n");
	}
	return false;
}

bool SemaphoreP(int semaphore) // blocks the process if S<=0
{							   // 0 to 4 sem ID

	// check
	if (!(semaphore >= 0 && semaphore < 5))
	{
		printf("Semphore ID not in bounds\n");
		return false;
	}
	if (sem[semaphore].created == false)
	{
		printf("Semaphore has not been intilized\n");
		return false;
	}
	if (Running->pid == 0)
	{
		printf("Init cannot be blocked so it cannot use Semaphores\n");
		return false;
	}

	if (sem[semaphore].val > 0) 
	{
		// decrement by 1 but do not block
		sem[semaphore].val--;
	}
	else
	{
		// need to block process
		Running->status = blocked;
		List_prepend(sem[semaphore].processesWaiting, Running); // add to the p block queue
		List_prepend(blockedALLsem, Running);
		// add old value to the queue
		Process *temp = getUnblockfromQueue();
		if (temp == NULL)
		{
			Running = init;
		}
		else
		{
			Running = temp;
		}
	}

	return true;
}

bool SemaphoreV(int semaphore)
{
	// // 0 to 4 sem ID
	if (!(semaphore >= 0 && semaphore < 5))
	{
		printf("Semphore ID not in bounds\n");
		return false;
	}
	if (sem[semaphore].created == false)
	{
		printf("Semaphore has not been intilized\n");
		return false;
	}

	sem[semaphore].val++; // increment the semaphore

	if (sem[semaphore].val > 0) // two cases now
	{

		Process *temp = (Process *)List_trim(sem[semaphore].processesWaiting); // unblock a process
		if (temp != NULL)
		{
			List_first(blockedALLsem);								// make first on list
			List_search(blockedALLsem, compareFunct, &(temp->pid)); // find pid from this
			List_remove(blockedALLsem);
			if (Running->pid == 0) // when unblocked check if it becomes the only non-init process
			{
				Running = temp;
				Running->status = running;
				sem[semaphore].val--;
				return true;
			}

			const int priority = temp->curPriority; // otherwise add the unblock process to the queue
			if (priority == 0)
				List_append(queue0, temp); // highest priorty
			else if (priority == 1)
				List_append(queue1, temp);
			else
				List_append(queue2, temp);
			temp->status = queued;
			sem[semaphore].val--;
		}
	}
	return true;
}

bool Procinfo(int pid) // prints the process of pid
{
	Process *process = NULL;
	List *queues[] = {queue0, queue1, queue2, blockQueue, blockedALLsem};
	for (int i = 0; i < 5; ++i)
	{
		List_first(queues[i]);
		process = List_search(queues[i], compareFunct, &pid);
		if (process != NULL)
			break;
	}
	if (pid == Running->pid)
	{
		process = Running;
	}
	// When process pid found in the queues
	if (process != NULL)
	{
		printProcess(process);

		return true;
	}
	else
	{
		printf("Process with PID %d not found.\n", pid);
		return false;
	}
}

void Totalinfo() // prints evey process that is allocated
{
	printf("Total Information:\n");
	printf("\n Running Process:\n");
	if (Running != NULL)
	{
		printProcess(Running);
	}
	else
	{
		printf("No running process\n");
	}

	printf("\n Priority 0 (High):\n");
	printQueueInfo(queue0);

	printf("\n Priority 1 (Normal):\n");
	printQueueInfo(queue1);

	printf("\n Priority 2 (Low):\n");
	printQueueInfo(queue2);
	printf("\n Blocked Messages:\n");
	printQueueInfo(blockQueue);
	printf("\n Blocked by semaphore:\n");
	printQueueInfo(blockedALLsem);
	if (Running->pid != 0)
	{
		printf("\n Init Process:\n");
		printProcess(init);
	}
	
}
