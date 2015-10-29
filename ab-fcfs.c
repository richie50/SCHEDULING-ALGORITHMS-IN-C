/*
# Family Name:	Abhishek
# Given Name:		Bhole
# Student Number:	210873867
# CSE Login:		cse13246
*/



#include <stdio.h>
#include <stdlib.h>
#include "sch-helpers.h"
#include <assert.h>
#define YES 0
#define NO 1

process processes[MAX_PROCESSES + 1];   // a large structure array to hold all processes read from data file
int numberOfProcesses = 0;
int cpusRunning = 0;

//Create CPU queue
typedef struct CPU
	{					// CPU information
	process *proc;		// stored process
	int isIdle;			// Indicates if CPU is idle.
	} CPU;

// Create Ready Queue, wait Queue (I/O), jobQueue

process_queue readyQueue, waitQueue, jobQueue, sortingQueue;

// Create an array of CPU structures (quad CPU)

CPU cpu[NUMBER_OF_PROCESSORS];

process_node *tempNode;

int main(int argc, char *argv[])
{
// general utility counting variables
	int i = 0, j = 0;		// basic counters used throughout program
	int CPUClock = 0;		// Simulated CPU clock
	int CPUsRunning = 0;	// Indicates the number of CPU's running at any given time
	int	turnAround = 0;		// used to calculate turnaround time
	int CPUUtilization = 0;	// used to calculate CPU utilization
	int totalWait = 0;		// holds the total wait time
	int exitPID[NUMBER_OF_PROCESSORS][2];		// holds id's of exiting processes along with their time

	// lets do some intialization of the CPU's
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
		cpu[i].isIdle = YES;
	}


// Loading all the data into the process structure
while ((i = readProcess(&processes[numberOfProcesses])))
{
	if(i==1)
		numberOfProcesses++;
}
// sorting algorithim to sort processes by arrival time
	qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

// Initialize the process queues

	initializeProcessQueue(&readyQueue);	// Processes ready to go to CPU
	initializeProcessQueue(&waitQueue);		// Processes waiting to go to CPU
	initializeProcessQueue(&jobQueue);		// Processes that have not yet arrived
	initializeProcessQueue(&sortingQueue);	// Processes that need to be sorted by PID
											// before entering the ready queue

// Add processes to ready queue
	for(i = 0; i < numberOfProcesses; i++)
		enqueueProcess(&jobQueue, &processes[i]);

	//Loop until there is no work left to be done
while(waitQueue.size + readyQueue.size + jobQueue.size + CPUsRunning > 0)
{
	//decrease length of burst by 1
	for (tempNode = waitQueue.front; tempNode != NULL; tempNode = tempNode->next)
		tempNode->data->bursts[tempNode->data->currentBurst].length--;

	// decrease the CPU burst step in each CPU by 1
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
		if(cpu[i].isIdle == NO)
			{
			cpu[i].proc->bursts[cpu[i].proc->currentBurst].length--;
			assert(cpu[i].proc->bursts[cpu[i].proc->currentBurst].length >= 0);
			}
	}
	// Increase the wait time of each processes in ready queue that was not
	// picked up by a CPU
	for (tempNode = readyQueue.front; tempNode != NULL; tempNode = tempNode->next)
					tempNode->data->waitingTime++;



	//Move from job to ready queue No need to increase currentBurst
	//already at 0 initial value

	for(tempNode = jobQueue.front; tempNode != NULL; tempNode = tempNode->next)
		{
			if(tempNode->data->arrivalTime == CPUClock)
				{
					tempNode->data->startTime = CPUClock;
					enqueueProcess(&sortingQueue, jobQueue.front->data);
					dequeueProcess(&jobQueue);
				}
			else if(tempNode->data->arrivalTime > CPUClock && jobQueue.size > 0)
					break;
		}


	// looping through the wait queue.  Since only can remove
	// from the front I move the processes that are not ready
	// to go into the ready queue to the back.  Do this waitQueue.size
	// times so that I preserve the order of the wait queue
	j = waitQueue.size;
	for(i = 0; i < j; i++)
	{
		if(waitQueue.front->data->bursts[waitQueue.front->data->currentBurst].length == 0)
		{
			waitQueue.front->data->currentBurst++;
			enqueueProcess(&sortingQueue, waitQueue.front->data);
			dequeueProcess(&waitQueue);
		}
		else
		{
			enqueueProcess(&waitQueue, waitQueue.front->data);
			dequeueProcess(&waitQueue);
		}
		if(waitQueue.size == 0)
			break;	//break out of this loop if I reach 0 before loop is finished
	}

	{	// Sorting all the process that arrived from the job queue
		// and waiting queue by PID's since these processes arrived
		// at the same time
		//if(sortingQueue.size > 1)
			//sortQueue(&sortingQueue);

		// enqueue these to the ready queue.
		j = sortingQueue.size;
		for(i = 0; i < j; i++)
		{
			enqueueProcess(&readyQueue, sortingQueue.front->data);
			dequeueProcess(&sortingQueue);
		}
	}


	// CPU control.  Loop through all the cpu's and increase the burst rate
	// for those processes that are finished move them to the waiting queue if
	// there are burst remaining.  Otherwise release the process and update
	// statistics.
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
	if (cpu[i].isIdle == NO && cpu[i].proc->bursts[cpu[i].proc->currentBurst].length == 0)
	{
		//If there are bursts remaining in the process then enqueue to wait queue
		if(cpu[i].proc->currentBurst < (cpu[i].proc->numberOfBursts-1))
		{
			cpu[i].proc->currentBurst++;
			enqueueProcess(&waitQueue, cpu[i].proc);
		}
		//If processes is finished, update statistics
		else if (cpu[i].proc->currentBurst == (cpu[i].proc->numberOfBursts -1))
		{
			cpu[i].proc->endTime = CPUClock;
			totalWait += cpu[i].proc->waitingTime;
			turnAround = turnAround + (cpu[i].proc->endTime - cpu[i].proc->startTime);
			exitPID[i][0] = cpu[i].proc->pid;
			exitPID[i][1] = CPUClock;
		}
		// release cpu
		cpu[i].isIdle = YES;
		CPUsRunning--;
	}
	// add another process to cpu
	if (cpu[i].isIdle == YES)
		{
		if (readyQueue.size > 0)
			{
			cpu[i].proc = readyQueue.front->data;
			dequeueProcess(&readyQueue);
			CPUsRunning++;
			cpu[i].isIdle = NO;
			}
		}
	}
	CPUUtilization += CPUsRunning;	/* calculate cpu utilization */
	CPUClock++;
}
	CPUClock--;
	printf("Average waiting time\t\t\t: %.2f\n",totalWait * 1. / numberOfProcesses);
	printf("Average turnaround time\t\t\t: %.2f\n", turnAround * 1. / numberOfProcesses);
	printf("Time all processes finished\t\t: %d\n",CPUClock);
	printf("Average CPU utilization\t\t\t: %.1f%%\n",CPUUtilization * 100. / CPUClock);
	printf("Number of context switches\t\t: %d\n", 0); //context switch not used in FCFS
	printf("PID(s) of last process(es) to finish\t: ");
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
		if(exitPID[i][1] == CPUClock)
			printf("(%d) ",exitPID[i][0]);
		printf("\n");
	return (0);
}
