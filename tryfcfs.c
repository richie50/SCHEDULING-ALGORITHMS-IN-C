/* 
 * File:   main.c
 * Author: RichMond
 *
 * Created on July 8, 2015, 8:15 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "sch-helpers.h"
/*
 * 
 */
#define TRUE 1

process processes[MAX_PROCESSES + 1];
process *CPUS[NUMBER_OF_PROCESSORS]; // process running on each cpu
process_queue ready_queue;
process_queue waiting_queue;
process *pre_ready_queue[MAX_PROCESSES]; //cant use . have to use ->
int pre_ready_queue_size;

void init(void) {
    int i = 0;
    while (i < NUMBER_OF_PROCESSORS) {
        CPUS[i] = NULL; //initialized all process running on each cpu
    }
    pre_ready_queue_size = 0;
    //initialized  queues
    initializeProcessQueue(&ready_queue);
    initializeProcessQueue(&waiting_queue);
}

/*const void points to a memory loc that u dont wana modify*/
int compare_process(const void *one, const void *two) {
    process *p1 = *((process **) one); //cast the struct object
    process *p2 = *((process **) two);
    if (p1->pid < p2->pid) {
        return -1;
    }
    if (p1->pid > p2->pid) {
        return 1;
    }
    return 0;
}

int get_current_running_process(void) {
    int result = 0;
    int j = 0;
    while (j < NUMBER_OF_PROCESSORS) {
        if (CPUS[j] != NULL) {
            result++;
        }
        j++;
    }
    return result;
}

process *get_next_schedule_process(void);

int main(int argc, char** argv) {
    int process_complete = 0;
    int number_of_processes = 0;

    while ((process_complete = readProcess(&processes[number_of_processes]))) {
        if (process_complete == 1) {
            number_of_processes++;
        }
        if (number_of_processes > MAX_PROCESSES) { // if there is no more processes
            break;
        }
    }
    printf("MAX PROCESS:%d\tCurrent Processes:%d\n", MAX_PROCESSES, number_of_processes);
    if (number_of_processes == 0) {
        fprintf(stderr, "No processes specified in input.\n");
    } else {
        if (number_of_processes > MAX_PROCESSES) {
            fprintf(stderr, "Too processes specified in input. MAX is %d\n", MAX_PROCESSES);
        }
    }
    int i = 0;
    int j = 0;
    FILE *out;
    out = fopen("out.txt", "w+");
    if (out == NULL) return -1;

    for (i = 0; i < number_of_processes; i++) {
        /*DEBUG PRINTING*/
        char *s = "------------------------------------------------------------------------\n";
        fprintf(out, "%s", s);
        fprintf(out, "PID:%d\tArrivalTime:%d\t", processes[i].pid, processes[i].arrivalTime);
        //printf("CurrentBurst for each process:%d\n", processes[i].currentBurst);
        //printf("NUmber of burst struct:%d\n", processes[i].numberOfBursts);
        for (j = 0; j < MAX_BURSTS; j++) {
            fprintf(out, "CPU Burst:%d\t", processes[i].bursts[j].length);
            j++;
            fprintf(out, "(I/OBurst):%d\t", processes[i].bursts[j].length);

        }
        fprintf(out, "\n");
        char *ss = "*****************************************************************************\n";
        fprintf(out, "%s", ss);
    }

    fclose(out);
    /*Incoming processes*/
    int glob = 0;
    int next_incoming_process = 0;
    int default_time = 0;
    while (TRUE) {
        glob++;
        /*if (glob == 7000) {
            break;
        }*/

        while (next_incoming_process < number_of_processes && processes[next_incoming_process].arrivalTime <= default_time) {
            pre_ready_queue[pre_ready_queue_size] = &processes[next_incoming_process];
            printf("HOW MANY TIMES\n");
            pre_ready_queue_size++;
            next_incoming_process++;
        }
        /*Move cpu that has finish it work to the waiting queue*/
        j = 0;
        while (j < NUMBER_OF_PROCESSORS) {
            if (CPUS[j] != NULL) {
                printf("THE PROCESS ID IN THE CPU:%d\tArrivalTime:%d\n", CPUS[j]->pid, CPUS[j]->arrivalTime);
                printf("Current Burst for each process:%d\n", CPUS[j]->currentBurst);
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
                        CPUS[j]->endTime = default_time;
                    }
                     //CPUS[j] == NULL; /*Stop current process*/      
                }
            }else{
                printf("first time  CPU has nothing\n");
            }
            j++;
        }
        //move waiting queue to ready queue
        int k = 0;
        int waiting_q_size = waiting_queue.size;
        for (k = 0; k < waiting_q_size; k++) { //from waiting to pre ready then to CPU
            printf("Waiting queue Size:%d\tITS PID:%d\n", waiting_q_size, waiting_queue.front->data->pid);
            process *waiting_process = waiting_queue.front->data; //? is it like process[at some index]????? is NULL
            dequeueProcess(&waiting_queue);
            if(waiting_process->bursts[waiting_process->currentBurst].step == waiting_process->bursts[waiting_process->currentBurst].length){
                waiting_process->currentBurst++; //gets the next burst assuming cpu
                pre_ready_queue[pre_ready_queue_size++] = waiting_process;
                 printf("HEREEEEEE------>>>>\t%p\t%p\n", pre_ready_queue[pre_ready_queue_size++], waiting_process);
            }else{
                enqueueProcess(&waiting_queue, waiting_process);
            }
        }

        /*put ready process in the cpu , sort pre ready queue to determining the process with highest priority using its pid*/
        qsort(pre_ready_queue, pre_ready_queue_size, sizeof (process*), compare_process);
        /*We put the most ready process in the pre ready queue */
        printf("BEFORE ENQUEUE->%d\n" , pre_ready_queue_size);
        for(i = 0; i < pre_ready_queue_size ; i++){
            printf("ENQUEUE\n");
            enqueueProcess(&ready_queue, pre_ready_queue[i]);
        }
        pre_ready_queue_size = 0; // WHY?????????
        int ready_q_size = ready_queue.size;
        printf("rq size:%d\tpreReadySize:%d\n", ready_q_size , pre_ready_queue_size);
        for (k = 0; k < NUMBER_OF_PROCESSORS; k++) {
            if (CPUS[k] == NULL) {
                CPUS[k] = get_next_schedule_process(); // gave cpu a job
                printf("CPU->%p\n" ,CPUS[k]); 
            }
        }
        //update
        for (k = 0; k < waiting_queue.size; k++) {
            process *front_data = waiting_queue.front->data;
            dequeueProcess(&waiting_queue);
            //front_data->bursts[front_data->currentBurst].step++;
            enqueueProcess(&waiting_queue, front_data);
        }
         for (k = 0; k < ready_queue.size; k++) {
            process *front_data = ready_queue.front->data;
            dequeueProcess(&ready_queue);
            front_data->waitingTime++;
            enqueueProcess(&waiting_queue, front_data);
        }
        for( j = 0; j < NUMBER_OF_PROCESSORS ; j++){
            if(CPUS[j] != NULL){
                //increase step
                CPUS[j]->bursts[CPUS[j]->currentBurst].step++;
            }
        }
        
        printf("Default:%d\tnumberofProcess:%d\tnextincomingprocess:%d\n", default_time , number_of_processes , next_incoming_process);
        int get = get_current_running_process();
        //int get = 0;
        printf("CURRENT CPU RUNNING:%d\n" , get);
        if((number_of_processes - next_incoming_process) == 0){
            //if(waiting_q_size == 0){
                break; //no more process to run
            //}
        }
        default_time++;
        fflush(stdout);
    }
    //compute
    return (EXIT_SUCCESS);
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
