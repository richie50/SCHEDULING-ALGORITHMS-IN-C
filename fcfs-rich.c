#include <stdio.h>
#include <stdlib.h>
#include "sch-helpers.h"
#define TRUE 1

process processes[MAX_PROCESSES + 1];
process *CPUS[NUMBER_OF_PROCESSORS]; // process running on each cpu
process_queue ready_queue;
process_queue waiting_queue;

int compare_process_arrivals(const void *one, const void *two);
void init(void);
process *get_next_schedule_process(void);

int main(int argc, char** argv) {
	init();
    int process_complete = 0;
    int number_of_processes = 0;
	int process_size = 0;
	int current_prc = 0;
    while ((process_complete = readProcess(&processes[number_of_processes]))) {
        if (process_complete == 1) {
            number_of_processes++;
        }
        if (number_of_processes > MAX_PROCESSES) { // if there is no more processes
            break;
        }
    } /*end of reading and parsing input data*/
	printf("MAX PROCESS:%d\tCurrent Processes:%d\n", MAX_PROCESSES, number_of_processes);
    if (number_of_processes == 0) {
        fprintf(stderr, "No processes specified in input.\n");
    } else {
        if (number_of_processes > MAX_PROCESSES) {
            fprintf(stderr, "Too processes specified in input. MAX is %d\n", MAX_PROCESSES);
        }
    }
	qsort(processes, process_size , sizeof (process*), compare_process_arrivals);
	int j = 0;
	int total_time = 0;
	while( j < number_of_processes){
		total_time = total_time + processes[j].arrivalTime;
		j++;
	}
	printf("Total arrival time:%d\n" , total_time);
	int simulation_time = 0;
	
	//RUN STIMULATION
	while(TRUE){
		
		simulation_time++;
		if(simulation_time == 100){
			break;
		}
		j = 0;
		while (j < NUMBER_OF_PROCESSORS) {
            if (CPUS[j] != NULL) {
				printf("CPU CURRENT PID->%d\n" ,CPUS[CPUS[j]->currentBurst]->pid); 
                printf("CPU BURST && I/O BURST:%d\t",CPUS[j]->bursts[CPUS[j]->currentBurst].length );
                printf("STEPS:%d\n" , CPUS[j]->bursts[CPUS[j]->currentBurst].step);
                if (CPUS[j]->bursts[CPUS[j]->currentBurst].step == CPUS[j]->bursts[CPUS[j]->currentBurst].length) {
                    /*if there is a process in the CPU then updates its information*/
                    /*and the add it to the waiting queue if its not finished its work*/
                    printf("***************DONEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE************************\n");
                    CPUS[j]->currentBurst++; //go over all I\O CPU burst
                    if (CPUS[j]->currentBurst < CPUS[j]->numberOfBursts) { /*iterate over all  I/O bound and CPU bound burst*/
                        /*data init from file->cpudat.txt*/
                        enqueueProcess(&waiting_queue, CPUS[j]); /*puts a waiting process in a waiting queue*/
                        printf("************************************************************************\n");
                    } else {
                        /*current cpu has finish its work*/
                        CPUS[j]->endTime = simulation_time;
                    }      
                }
            }else{
                //printf("first time  CPU has nothing\n");
            }
            j++;
        }
		           
        enqueueProcess(&ready_queue, &processes[current_prc]);
        printf("ENQUEUE PID:%d\n", ready_queue.front->data->pid);
		
		int ready_q_size = ready_queue.size;
        printf("rq size:%d\n", ready_q_size);
		int x = 0;
        for (x = 0; x < NUMBER_OF_PROCESSORS; x++) {
            if (CPUS[x] == NULL) {
                CPUS[x] = get_next_schedule_process(); // gave cpu a job
                // dequeue ready process because it has run already
				process_size++; // current processes already executed
            }
        }
		//put current process in the waiting queue after is has gone in the ready queue
		//for( j = 0 ; j < process_size; j++){ //enqueue already runned process in the waiting queue 
			enqueueProcess(&waiting_queue , &processes[current_prc]);
		//}
		current_prc++;
		printf("Waiting queue waiting for I/O bound stuff:%d\n" , waiting_queue.size);
		printf("%d\n" , simulation_time);
	} //end simulation
	//calculating waiting time
	int i , k , waiting_time;
	waiting_time  = processes[0].arrivalTime; //
	for(i = 1; i < number_of_processes;i++){
		for(k = 0; k < MAX_BURSTS;k++){
			waiting_time = waiting_time + processes[i].bursts[k].length;// CPU BURST
			waiting_time = waiting_time + processes[i].bursts[k++].length; //I/O BURST
		}
	}
	printf("PID:%d\tWaitingTime:%d\n" , processes[i].pid , waiting_time);
	return 0;
}
void init(void) {
    int i = 0;
    while (i < NUMBER_OF_PROCESSORS) {
        CPUS[i] = NULL; //initialized all process running on each cpu
		i++;
    }
    //initialized  queues
    initializeProcessQueue(&ready_queue);
    initializeProcessQueue(&waiting_queue);
}
int compare_process_arrivals(const void *one, const void *two) {
    process *p1 = *((process **) one); //cast the struct object
    process *p2 = *((process **) two);
    if (p1->arrivalTime < p2->arrivalTime) {
        return -1;
    }
    if (p1->arrivalTime > p2->arrivalTime) {
        return 1;
    }
    return 0;
}
process *get_next_schedule_process(void){
    if(ready_queue.size == 0){
        return NULL;
    }
    process *next_process = ready_queue.front->data;
    printf("PID IN FUNC:%d\n" , next_process->pid);
    dequeueProcess(&ready_queue);
    return next_process;
}