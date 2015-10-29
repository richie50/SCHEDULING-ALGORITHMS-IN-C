/* 
 * File:   fbq.c
 * Author: RICHMOND FRIMPONG
 * CSE23004
 * 212114641
 * CSE3221 SUMMER 2015 SECTION Z
 * Created on July 17, 2015, 1:43 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "sch-helpers.h"

#define TRUE 1
#define INT32_MAX 0xffffffff
#define MAX_TIME 40
FILE *log_file;

process processes[MAX_PROCESSES + 1];
int number_of_processes;
int incoming_process;
int context_switches;
int simulation_time;
int last_process_end_time;
int cpu_utilized_time;
int time_slice;
int time_slice_1;
process_queue readyQueue;
process_queue waitingQueue;
process_queue level_one;
//process_queue promoted;
process_queue second_level;
process *mfbq_queue1[NUMBER_OF_PROCESSORS];
process *mfbq_queue2[NUMBER_OF_PROCESSORS];
process *promoted_queue[NUMBER_OF_PROCESSORS];
process *promoted_queue_2[NUMBER_OF_PROCESSORS];
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
    int i, j, k, l, m, x;
    j = 0;
    k = 0;
    l = 0;
    m = 0;
    int rem_time = 0;
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (CPU[i] != NULL) {
            fprintf(log_file, "STEPS:%d\n", CPU[i]->bursts[CPU[i]->currentBurst].step);
            // at some point step will be equal to length         
            if (CPU[i]->bursts[CPU[i]->currentBurst].step == CPU[i]->bursts[CPU[i]->currentBurst].length) {
                fprintf(log_file, "DONE PROCESS ID:%d\t && RT\t:%d\n", CPU[i]->pid, rem_time);
                CPU[i]->currentBurst++;
                if (CPU[i]->currentBurst < CPU[i]->numberOfBursts) {
                    enqueueProcess(&waitingQueue, CPU[i]); // Puts the finish Burst into the waiting queue
                    fprintf(log_file, "Waiting process id:%d\t&& Current Process id:%d\n", waitingQueue.front->data->pid, CPU[i]->pid);
                } else {
                    CPU[i]->endTime = simulation_time;
                    last_process_end_time = CPU[i]->endTime;
                }
                CPU[i] = NULL;
                //first level
            } else if (CPU[i]->quantumRemaining == 0 && CPU[i]->currentQueue == 0) {
                // process that doesn't end with the 
                //given time slices 
                fprintf(log_file, "first time slice is met\n");
                fprintf(log_file, "PID:%d\tLength:%d\n", CPU[i]->pid, CPU[i]->bursts[CPU[i]->currentBurst].length);
                // give that process the second time slice
                CPU[i]->quantumRemaining = time_slice_1;
                CPU[i]->currentQueue = 1;
                mfbq_queue1[j] = CPU[i];
                context_switches++;
                j++;
                CPU[i] = NULL; //free the CPU for another process
                //second level
            } else if (CPU[i]->quantumRemaining == 0 && CPU[i]->currentQueue == 1) {
                //doesn't finish within those two time slices
                fprintf(log_file, "Second time slice is met\n");
                fprintf(log_file, "PID:%d\tLength:%d\n", CPU[i]->pid, CPU[i]->bursts[CPU[i]->currentBurst].length);
                CPU[i]->quantumRemaining = INT32_MAX; // the maximum time 
                //CPU[i]->quantumRemaining = MAX_TIME;
                CPU[i]->currentQueue = 2;
                context_switches++;
                mfbq_queue2[k] = CPU[i];
                k++;
                CPU[i] = NULL; //free the cpu for another process
                //Promotion
            } else if (CPU[i]->bursts[CPU[i]->currentBurst].length < time_slice) {
                fprintf(log_file, "Promotion 1 defaults to FCFS . . . . .\n");
                fprintf(log_file, "PID:%d\tLength:%d\n", CPU[i]->pid, CPU[i]->bursts[CPU[i]->currentBurst].length);
                CPU[i]->currentQueue = 0; // priority level remains unchanged 
                promoted_queue[l] = CPU[i];
                l++;
                CPU[i] = NULL;
            } else if (CPU[i]->bursts[CPU[i]->currentBurst].length < time_slice_1) {
                fprintf(log_file, "Promotion 2 defaults to FCFS . . . .\n");
                fprintf(log_file, "PID:%d\tLength:%d\n", CPU[i]->pid, CPU[i]->bursts[CPU[i]->currentBurst].length);
                CPU[i]->currentQueue = 0; // priority level remains unchanged 
                promoted_queue_2[m] = CPU[i];
                m++;
                CPU[i] = NULL;
            }
        }
    }
    /* Add all interrupt process back to various queues depending on their priority*/
    qsort(mfbq_queue1, j, sizeof (process*), compareProcessIds);
    for (x = 0; x < j; x++) {
        enqueueProcess(&level_one, mfbq_queue1[x]);
    }
    qsort(mfbq_queue2, k, sizeof (process*), compareProcessIds);
    for (x = 0; x < k; x++) {
        enqueueProcess(&second_level, mfbq_queue2[x]);
    }
    qsort(promoted_queue, l, sizeof (process*), compareProcessIds);
    for (x = 0; x < l; x++) {
        enqueueProcess(&readyQueue, promoted_queue[x]);
    }
    qsort(promoted_queue_2, m, sizeof (process*), compareProcessIds);
    for (x = 0; x < m; x++) {
        enqueueProcess(&readyQueue, promoted_queue_2[x]);
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
    process *cpu_ready;
    if (readyQueue.size > 0) {
        cpu_ready = readyQueue.front->data;
        dequeueProcess(&readyQueue);
        return cpu_ready;
    } else if (level_one.size > 0) {
        cpu_ready = level_one.front->data;
        dequeueProcess(&level_one);
        return cpu_ready;
    } else if (second_level.size > 0) {
        cpu_ready = second_level.front->data;
        dequeueProcess(&second_level);
        return cpu_ready;
    } else {
        return NULL;
    }
}

/**
 * initialize the processes to a temporary array
 */
void incoming_process_init() {
    while (incoming_process < number_of_processes && processes[incoming_process].arrivalTime <= simulation_time) {
        tmp_ready_process[tmp_ready_process_size] = &processes[incoming_process];
        tmp_ready_process[tmp_ready_process_size]->currentQueue = 0; //first level initialization
        tmp_ready_process[tmp_ready_process_size]->quantumRemaining = time_slice;
        tmp_ready_process_size++;
        incoming_process++;
    }
}

/**
 * 
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

/* update processes in the waiting queue */
void waiting_to_ready() {
    int i = 0;
    int waiting_size = waitingQueue.size;
    for (i = 0; i < waiting_size; i++) {
        process *ready = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        if (ready->bursts[ready->currentBurst].step == ready->bursts[ready->currentBurst].length) {
            ready->currentBurst++;
            if (ready->bursts[ready->currentBurst].length < time_slice) { //defaults to fcfs
                ready->quantumRemaining = INT32_MAX;
            } else if (ready->bursts[ready->currentBurst].length < time_slice_1) { //defaults to fcfs
                ready->quantumRemaining = INT32_MAX;
            }
            if (ready->currentQueue == 0) {
                ready->quantumRemaining = time_slice;
                //context_switches++;
            } else if (ready->currentQueue == 1) {
                ready->quantumRemaining = time_slice_1;
                //context_switches++;
            } else {
                ready->quantumRemaining = INT32_MAX;
                context_switches++; // processes time didnt not expire so context switches comes from I/O(waiting queue) instead
            }
            tmp_ready_process[tmp_ready_process_size++] = ready;
        } else {
            enqueueProcess(&waitingQueue, ready);
        }
    }
}

/* updates the waiting time of the CPU */
void refresh_processes() {
    int j;
    // update CPU BOUND waiting process 
    for (j = 0; j < readyQueue.size; j++) {
        process *CPU_BOUND = readyQueue.front->data;
        dequeueProcess(&readyQueue);
        CPU_BOUND->waitingTime++;
        enqueueProcess(&readyQueue, CPU_BOUND);
    }
}
// increases work done by either I/O or CPU bound processes and decrease quantum time

void increase_cpu_work() {
    int j;
    for (j = 0; j < NUMBER_OF_PROCESSORS; j++) {
        if (CPU[j] != NULL) {
            CPU[j]->bursts[CPU[j]->currentBurst].step++;
            CPU[j]->quantumRemaining--;
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
    if (argc < 3) {
        fprintf(stderr, "Please enter a time quantums . . . . \n");
        exit(1);
    }
    time_slice = atoi(argv[1]);
    time_slice_1 = atoi(argv[2]);
    if (time_slice <= 0 || time_slice_1 <= 0) {
        fprintf(stderr, "Error! program usage rr < positive time quantum> < data file . . .\n");
        exit(1);
    }
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
    //initialize cpu's
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        CPU[i] = NULL;
    }
    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
    initializeProcessQueue(&level_one);
    initializeProcessQueue(&second_level);
    //initializeProcessQueue(&promoted);
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
        // break when there are no more running or incoming processes, and the waiting queue is empty
        if (remaining_process == 0 && runningProcesses() == 0 && waitingQueue.size == 0) {
            break;
        }
        simulation_time++;
    }
    int total_waiting_time = 0;
    int turn_around_time = 0;
    for (i = 0; i < number_of_processes; i++) {
        turn_around_time += processes[i].endTime - processes[i].arrivalTime;
        total_waiting_time += processes[i].waitingTime;
    }
    printf(">>>>>>>>>>>>> FBQ with Q1 :%d\tQ2 :%d  <<<<<<<<<<<<<<<\n", time_slice, time_slice_1);
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
