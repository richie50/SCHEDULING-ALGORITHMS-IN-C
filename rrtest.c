/* 
 * File:   rr.c
 * Author: RichMond
 *
 * Created on July 17, 2015, 1:43 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "sch-helpers.h"

#define TRUE 1
FILE *log_file;

process processes[MAX_PROCESSES + 1];
int number_of_processes;
int incoming_process;
int context_switches;
int simulation_time;
int last_process_end_time = 0;
int cpu_utilized_time;
int time_slice = 0;
process_queue readyQueue;
process_queue waitingQueue;
process *request_queue[NUMBER_OF_PROCESSORS];

// CPU's for Quadcore simulation
process *CPU[NUMBER_OF_PROCESSORS];

// Temporary "Before-Ready" queue
process *tmp_ready_process[MAX_PROCESSES + 1];
int tmp_ready_process_size;

void init_(void) {
    number_of_processes = 0;
    incoming_process = 0;
    context_switches = 0;
    cpu_utilized_time = 0;
    simulation_time = 0;
    tmp_ready_process_size = 0;
}

int compareProcessIds(const void *a, const void *b) {
    process *one = *((process**) a);
    process *two = *((process**) b);
    if (one->pid < two->pid)
        return -1;
    if (one->pid > two->pid)
        return 1;

    return 0;
}

/**
 * Move finish process into the waiting queue and terminate finish CPU or I/O bursts
 */
void running_process_to_waiting() {
    int i, k;
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (CPU[i] != NULL) {
            //printf("i\t:%d\n" , i);
            //fprintf(log_file, "CPU BURST && I/O BURST:%d\t", CPU[i]->bursts[CPU[i]->currentBurst].length);
            fprintf(log_file, "STEPS:%d\n", CPU[i]->bursts[CPU[i]->currentBurst].step);
            // Run according to time slice
            if (CPU[i]->bursts[CPU[i]->currentBurst].step == time_slice) {
                int rem_time = CPU[i]->bursts[CPU[i]->currentBurst].length - CPU[i]->bursts[CPU[i]->currentBurst].step;
                CPU[i]->bursts[CPU[i]->currentBurst].length = rem_time;
                CPU[i]->quantumRemaining = rem_time;
                request_queue[i] = CPU[i]; // put the next process in the request_queue
                //reset the step back to 0
                CPU[i]->bursts[CPU[i]->currentBurst].step = 0;
                fprintf(log_file, "REQUEST QUEUE ID:%d\tREDUCED INTIAL CPU BURST:%d\n", CPU[i]->pid, CPU[i]->bursts[CPU[i]->currentBurst].length);
                //now sort process in request queue by processes Id's
                printf("request queue i\t:%d\n" , i);
                //for (k = 0; k < i; k++) {
                    //enqueueProcess(&readyQueue, request_queue[k]);
                //}
				//qsort(request_queue, i, sizeof (process*), compareProcessIds);
				context_switches++;
            }
            // at some point step will be equal to length because length gets reduced
            if (CPU[i]->bursts[CPU[i]->currentBurst].step == CPU[i]->bursts[CPU[i]->currentBurst].length) {
                fprintf(log_file, "DONE PROCESS ID:%d\t && LENGTH\t:%d\n", CPU[i]->pid, rem_time);
                CPU[i]->currentBurst++;
                if (CPU[i]->currentBurst < CPU[i]->numberOfBursts) {
                    enqueueProcess(&waitingQueue, CPU[i]); // Puts the finish Burst into the waiting queue
                    fprintf(log_file, "Waiting process id:%d\t&& Current Process id:%d\n", waitingQueue.front->data->pid, CPU[i]->pid);
                } else {
                    CPU[i]->endTime = simulation_time;
                    printf("PID:%d\tET:%d\n", CPU[i]->pid, simulation_time);
                    last_process_end_time = CPU[i]->endTime;
                    //dequeueProcess(&request_queue); // 
                }
                CPU[i] = NULL;
            }
        }
    }
}

/**
 * Returns the current number of cpus running
 */
int runningProcesses() {
    int runningProcesses = 0;
    int i;
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (CPU[i] != NULL) {
            runningProcesses++;
        }
    }
    return runningProcesses;
}

/**
 * Gets the next ready process to be fed into the Cpu
 */
process *get_next_sch_process() {
    if (readyQueue.size == 0) {
        return NULL;
    }
    process *cpu_ready = readyQueue.front->data;
    //fprintf(log_file, "GIVEN TO CPU:%d\t && qt:%d\n", cpu_ready->pid, cpu_ready->quantumRemaining);
    dequeueProcess(&readyQueue);
    return cpu_ready;
}

/**
 * initialize the processes to a temporary array
 */
void incoming_process_init() {
    while (incoming_process < number_of_processes && processes[incoming_process].arrivalTime <= simulation_time) {
        tmp_ready_process[tmp_ready_process_size++] = &processes[incoming_process++];
    }
}

/**
 * Gets the most ready process and puts it the Cpu for execution
 */
void most_ready_running_in_cpu() {
    int i = 0;
    qsort(tmp_ready_process, tmp_ready_process_size, sizeof (process*), compareProcessIds);
    for (i = 0; i < tmp_ready_process_size; i++) {
        enqueueProcess(&readyQueue, tmp_ready_process[i]);
    }
    // at this point we feed the pre ready process into the CPU
    tmp_ready_process_size = 0;

    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (CPU[i] == NULL) {
            CPU[i] = get_next_sch_process();
        }
    }
}

void waiting_to_ready() {
    int i = 0;
    int waiting_size = waitingQueue.size;
    for (i = 0; i < waiting_size; i++) {
        process *ready = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        //dequeueProcess(&request_queue);
        if (ready->bursts[ready->currentBurst].step == ready->bursts[ready->currentBurst].length) {
            ready->currentBurst++;
            tmp_ready_process[tmp_ready_process_size++] = ready;
        } else {
            enqueueProcess(&waitingQueue, ready);
        }
    }
}

void refresh_processes() {
    int j;
    // update CPU BOUND waiting process 
    for (j = 0; j < readyQueue.size; j++) {
        process *CPU_BOUND = readyQueue.front->data;
        CPU_BOUND->waitingTime++;
		dequeueProcess(&readyQueue);
        if (CPU_BOUND->quantumRemaining == 0) {
            printf("*****DONE****\n");
            fprintf(log_file, "WAIITING TIME FOR REQUEST ID:%d\tis\t:%d\n", CPU_BOUND->pid, CPU_BOUND->waitingTime);
        } else {
            if (CPU_BOUND->quantumRemaining < 0) {
                dequeueProcess(&readyQueue);
                break;
            }
            //printf("Should i increase the waiting time for process that goes back into the request queue??\n");
        }
        enqueueProcess(&readyQueue, CPU_BOUND);
    }
}
// increases work done by either I/O or CPU bound processes

void increase_cpu_work() {
    int j;
    for (j = 0; j < NUMBER_OF_PROCESSORS; j++) {
        if (CPU[j] != NULL) {
            CPU[j]->bursts[CPU[j]->currentBurst].step++;
        }
    }
}

void increase_io_work() {
    int j;
    int size = waitingQueue.size;
    // update waiting state I\O burst
    for (j = 0; j < size; j++) {
        process *I_O = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        I_O->bursts[I_O->currentBurst].step++;
        enqueueProcess(&waitingQueue, I_O);
    }
}

int ex() {
    return number_of_processes - incoming_process;
}

int main(int argc, char **argv) {
    time_slice = atoi(argv[1]);
    printf("Time Slice:%d\n", time_slice);
    init_();
    clock_t ticks;
    time_t start_time, end_time;
    time(&start_time);
    int i;
    int status = 0;
    log_file = fopen("log_file.txt", "w+");
    if (log_file == NULL) {
        fprintf(stderr, "LOG FILE CANNOT BE OPEN\n");
    }
    // initialize cpus
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        CPU[i] = NULL;
    }

    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);

    // read in process and initialize process values
    while ((status = (readProcess(&processes[number_of_processes])))) {
        if (status == 1) {
            number_of_processes++;
        }

    }
    if (number_of_processes > MAX_PROCESSES) {
        return -2;
    }
    if (number_of_processes == 0) {
        return -1;
    }
    int remaining_process = 0;
    //sort process by their arrival times
    qsort(processes, number_of_processes, sizeof (process), compareByArrival);
    // main execution loop
    while (TRUE) {
        ticks = clock();
        waiting_to_ready();
        incoming_process_init();
        running_process_to_waiting();
        most_ready_running_in_cpu();


        refresh_processes();
        increase_io_work();
        increase_cpu_work();

        cpu_utilized_time += runningProcesses();
        remaining_process = ex();
        //printf("%d\t%d\t%d\n" , remaining_process ,waitingQueue.size , cpu_utilized_time);
        // break when there are no more running or incoming processes, and the waiting queue is empty
        if (remaining_process == 0 && runningProcesses() == 0 && waitingQueue.size == 0) {
            break;
        }
        //printf("%d\t%d\t%d\n" , remaining_process ,waitingQueue.size , cpu_utilized_time);
        simulation_time++;
    }
    //printf("%d\n" , simulation_time);
    //processes[number_of_processes - 13].endTime = simulation_time;
    // calculations based on CPU simulations
    int total_waiting_time = 0;
    int turn_around_time = 0;
    for (i = 0; i < number_of_processes; i++) {
        turn_around_time += processes[i].endTime - processes[i].arrivalTime;
        total_waiting_time += processes[i].waitingTime;
    }
    printf("********************************************************************\n");
    printf("Average Waiting Time\t\t\t:%.2f\n", total_waiting_time / (double) number_of_processes);
    printf("Average Turn Around Time\t\t:%.2f\n", turn_around_time / (double) number_of_processes);
    printf("Time all for all CPU processes\t\t:%d\n", simulation_time);
    printf("CPU Utilization Time\t\t\t:%.2f%c\n", (double) (cpu_utilized_time * 100.0) / (double) (simulation_time), (int) 37);
    printf("Total Number of Context Switches\t:%d\n", context_switches);
    printf("Last Process to finish ");
    for (i = 0; i < number_of_processes; i++) {
        if (processes[i].endTime == simulation_time) {
            printf("PID\t\t:%d\n", processes[i].pid);
        }
    }
    printf("********************************************************************\n");
    time(&end_time);
    double prg_time = (end_time - start_time) * 0.001;
    double cpu_time = (double) ticks / CLOCKS_PER_SEC;
    fprintf(log_file, "Program Time\t:%.2fsecs\n", prg_time);
    fprintf(log_file, "CPU Time\t:%.2fsecs\n", cpu_time);
    fclose(log_file);
    return 0;
}
