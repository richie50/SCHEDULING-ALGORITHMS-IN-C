/*
 # Family Name: Javed
 # Given Name: Daniyal
 # Student Number: 212654570
 # CSE Login: cse31034
 # Section: Z
*/
#include <stdio.h>
#include "sch-helpers.h"

int numProcess = 0;

process processes[MAX_PROCESSES + 1];   
process_queue ready_q;
process_queue device_q;
process *cpu[NUMBER_OF_PROCESSORS]; 
process *previousProc;

process *tempProccess[MAX_PROCESSES + 1];    
int proccessSize = 0;
double controlUnit = 0;
double taTime = 0;
double waitingTime = 0; // from sch=helpers.c
int contextSwitches = 0; 

/*compare by PID method, modified changed from sch-helpers.c*/

int compare(const void *m, const void *n) {

    process *a = *((process**) m);
    process *b = *((process**) n);
	
    if (a->pid < b->pid) return -1;
    if (a->pid > b->pid) return 1;
    return 0;
}

int main(){

	int place;
	while (place = readProcess(&processes[numProcess])){
      	   if(place == 1)  
		 numProcess ++;
	}
	
	/* sorting */ 
	qsort(processes, numProcess, sizeof(process), compareByArrival);

	int i=0;
	while(i < NUMBER_OF_PROCESSORS){
		cpu[i] = NULL;	
		i++;
	}

	initializeProcessQueue(&ready_q);
	initializeProcessQueue(&device_q);
	
	
	int ticker;
	int allProc = numProcess;
	
	for (ticker = 0;  allProc > 0; ticker++){
		//printf("%d\n" , ticker);
		/*ready Q*/
		int i = 0;
		while(i < numProcess){
			if(processes[i].arrivalTime == ticker){
				tempProccess[proccessSize] = &processes[i];
				proccessSize++;
			}
		i++;
		}
		
		
		/*CPU's if they are empty*/
		for(i = 0; i < NUMBER_OF_PROCESSORS; i++){		
		if (cpu[i] != NULL){
				/*CPU burst is ending*/
				if(cpu[i]->bursts[cpu[i]->currentBurst].step == cpu[i]->bursts[cpu[i]->currentBurst].length){
					cpu[i]->currentBurst++; 
					if(cpu[i]->currentBurst < cpu[i]->numberOfBursts)
					{					
						/*put the burst in the  waiting queue,increase the burst and take it out of the cpu*/
						enqueueProcess(&device_q, cpu[i]);
					}
					else 
					{
					
					previousProc = cpu[i];
					previousProc->endTime = ticker;
					//cpu[i]->endTime = ticker;
					allProc--;
					//printf("Endtime:%d\tpid:%d\n" , cpu[i]->endTime , cpu[i]->pid);
					}
					cpu[i] = NULL;
				}
				else{

				cpu[i]->bursts[cpu[i]->currentBurst].step++;
				controlUnit++;
				}
			}
		}

		int size = device_q.size;
		for(i = 0; i < size; i++){
			
		process *waitingProcess = device_q.front->data;
		
		/*Dequeue process if it has finished*/	
		if(waitingProcess->bursts[waitingProcess->currentBurst].step  == waitingProcess->bursts[waitingProcess->currentBurst].length){
				
				waitingProcess->currentBurst++;
				dequeueProcess(&device_q);
				tempProccess[proccessSize] = waitingProcess;
				proccessSize++;	
			}

			else{
				waitingProcess->bursts[waitingProcess->currentBurst].step++;
				dequeueProcess(&device_q);
				enqueueProcess(&device_q, waitingProcess);
			}
		}
			
			qsort(tempProccess, proccessSize, sizeof(process*), compare);
			/*Put the processes sorted by their pid*/
			for(i = 0; i < proccessSize; i++)
				enqueueProcess(&ready_q, tempProccess[i]);
			proccessSize = 0;

			for(i = 0; i < NUMBER_OF_PROCESSORS && ready_q.size > 0; i++)	{
				if(cpu[i] == NULL)
				{
					process *ready = ready_q.front->data;
					dequeueProcess(&ready_q);

					if(ready->startTime == 0)
						ready->startTime = ticker;
					ready->bursts[ready->currentBurst].step++;
					cpu[i] = ready;
					controlUnit++;
				}
			}
			
			size = ready_q.size;
			
			for(i = 0; i < size; i++){
					process *readyProcess = ready_q.front->data;
					readyProcess->waitingTime++;
					dequeueProcess(&ready_q);
					enqueueProcess(&ready_q, readyProcess);
			}
	}
	
	/*calculate waiting time*/
	int x = 0;
	while(x < numProcess){
		waitingTime = waitingTime + processes[x].waitingTime;
	x++;
	}
	
	/*Calculate turn around time*/
	for(x = 0; x < numProcess; x++){
		taTime = taTime + (processes[x].endTime - processes[x].arrivalTime);
	}
	
	printf("Average waiting time                 =  %.2f \n", waitingTime/numProcess );
	printf("Average turnaround time              =  %.2f \n", taTime / numProcess);
	printf("Time of all processes finished       =  %d\n", previousProc->endTime);
	printf("Average CPU utilization              =  %.1f\%\n", controlUnit/ticker * 100);
	printf("Number of context switches           =  %d\n", contextSwitches);
	printf("Last process to finish               =  %d\n", previousProc->pid);
	printf("****************************************************************\n");
	//int i = 0;
	for (i = 0; i < numProcess; i++) {
        if (processes[i].endTime == ticker) {
            printf("PID:%d\t\t\tENDTIME:%d\n", processes[i].pid, processes[i].endTime);
        }
    }
	return 0;
}
