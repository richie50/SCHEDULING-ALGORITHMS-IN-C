/* 
 * File:   sch-helpers.h
 * Author: user
 *
 * Created on July 10, 2015, 11:04 AM
 */

#ifndef SCH_HELPERS_H
#define	SCH_HELPERS_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Description: the header file for helper functions to be used for CSE3221 "scheduler" project

 (1) include this file in your main scheduler code as follows:

#include "sch-helpers.h"  

 (2) compile your codes as 

> gcc -o fcfs fcfs.c sch-helpers.c
> gcc -o rr rr.c sch-helpers.c
> gcc -o fbq fbq.c sch-helpers.c

***/

/* Some Macro Definitions */

#define printable(a) (((a) >= 32 && (a) <= 126) || (a) == '\t' || (a) == '\n')

#define MAX_PROCESSES 100      /* max number of processes per CPU load file */
#define MAX_BURSTS 1000        /* max number of bursts per process */
#define MAX_TOKEN_LENGTH 30
#define MAX_LINE_LENGTH (1<<16)

#define COMMENT_CHAR '#'
#define COMMENT_LINE -1

/*#define NUMBER_OF_PROCESSORS 4*/
#define NUMBER_OF_PROCESSORS 4 // for simulations purposes
/**
 * Struct definitions
 */

typedef struct burst burst;
typedef struct process process;
typedef struct process_node process_node;
typedef struct process_queue process_queue;

/* represents a CPU or IO burst */
struct burst {
    int length;                 /* number of time steps needed for burst */
    int step;                   /* number of time steps completed */
};

/* represents a process, containing all information pertaining to it,
   except for its state {new,ready,running,waiting,terminated}
   which is determined by which data structures (ready-Q, waiting-Q, cpu)
   it is associated with at a point in time. */
struct process {
    int pid;                    /* process id */
    int arrivalTime;            /* time process arrives in system */
    int startTime;              /* time process begins executing */
    int endTime;                /* time process finishes executing */
    int waitingTime;            /* total time process spends waiting */
    int currentBurst;           /* index of current burst in `bursts' array */
    int numberOfBursts;         /* number of bursts in `bursts' array */
    burst bursts[MAX_BURSTS];   /* alternating CPU and I/O bursts */
    int priority;               /* priority to order by before comparing pid (for RR only)*/
    int quantumRemaining;       /* time left in current time quantum (for RR only)*/
    int remainingTime;
    int currentQueue;           /* FBQ level the process is associated with (for FBQ only)*/
};

/* a node in a linked list of processes */
struct process_node {
   process *data;
    process_node *next;
};

/* FIFO queue implemented as a singly-linked list */
struct process_queue {
    int size;
    process_node *front;
    process_node *back;
};

/** Error management functions **/
void error(char *);
void error_malformed_input_line(char *);
void error_too_many_bursts(int);
void error_duplicate_pid(int pid);
void error_bad_quantum(void);

/** Queue management functions **/
process_node *createProcessNode(process *);
void initializeProcessQueue(process_queue *);
void enqueueProcess(process_queue *, process *);
void dequeueProcess(process_queue *);

/** Input/output functions **/
char *readLine(void);
char *readLineHelper(char *, int);
int readInt(char **);
int readBracedInt(char **);
int empty(char *);
int readProcess(process *);
int compareByArrival(const void *, const void *);



#ifdef	__cplusplus
}
#endif

#endif	/* SCH_HELPERS_H */

