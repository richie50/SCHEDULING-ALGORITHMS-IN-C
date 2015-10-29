
/**
 * Description: Some helper functions to be used for CSE3221 "scheduler" project
 */

/* Some hints to use these helper functions in this project */
/*  
   Step 1:  You should include the header file in your main scheduler code:

#include "sch-helpers.h"   

   Step 2: you should declare a global array in your own fcfs.c to hold info for all processes:

process processes[MAX_PROCESSES+1];   // a large structure array to hold all processes read from data file 
int numberOfProcesses=0;              // total number of processes 

   Step 3: you can call function readProcess() to read all data from stdio and sort the processes array ascending by arrival time:

... 
while (status=readProcess(&processes[numberOfProcesses]))  {
         if(status==1)  numberOfProcesses ++;
}   // it reads pid, arrival_time, bursts to one element of the above struct array
...
qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);
...

  Step 4: You may consider to use the following linked-list based Queue management functions to implement all scheduling queues (Ready Q and Device Q) for your scheduler:

process_node *createProcessNode(process *);
void initializeProcessQueue(process_queue *);
void enqueueProcess(process_queue *, process *);
void dequeueProcess(process_queue *);

  Step 5: After you are done, you can submit your fcfs.c as well as sch-helpers.h sch-helpers.c to the system. 
          Your code should compile as:

         $$ gcc -o fcfs fcfs.c sch-helpers.c

       In this case you can redirect all CPU load data to stdio when you run your FCFS schedule:

          $$ fcfs < CPULoad.dat    OR     $$ cat CPULoad.dat | fcfs 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "sch-helpers.h"     /* include a header file for function defintions and others */

/** Error management functions **/

/* print error message to stderr and terminate abnormally */ 
void error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(-1);
}

/* print error message displaying the first errant line in the input,
   or a special message in the presence of unprintable characters,
   and terminate abnormally. */
void error_malformed_input_line(char *line) {
    int i;

    /* handle lines with unprintable characters */
    for (i=0;i<strlen(line);i++) {
        if (!printable(line[i])) {
            line = "<cannot display line due to unprintable characters>\n";
            break;
        }
    }
    fprintf(stderr, "Error - malformed input line:\n%s\n"
                    "Expected integral values in range [0, 2^31-1], in format:\n"
                    "    \"pid arrival cpu-burst (io-burst) cpu-burst "
                    "(io-burst) ... cpu-burst\"\n"
                    "    (every process having between 1 and %d bursts, "
                         "ending on a CPU burst)\n",
                    line, MAX_BURSTS);
    exit(-1);
}

/* print appropriate error for the case wherein a process has too many bursts,
   then abnormally terminate. */
void error_too_many_bursts(int pid) {
    fprintf(stderr, "Error - too many bursts provided for process with id %d.\n"
                    "Total number of CPU or I/O bursts must not exceed %d (or %d combined).\n",
                    pid, MAX_BURSTS/2, MAX_BURSTS);
    exit(-1);
}

/* print appropriate error for the case wherein a process id appears more than
   once in the input, then abnormally terminate. */
void error_duplicate_pid(int pid) {
    fprintf(stderr, "Error - more than one process has id %d.\n", pid);
    exit(-1);
}

/* print appropriate error for the case wherein a time quantum specified in the
   command line arguments is invalid. */
void error_bad_quantum(void) {
    printf("Invalid time quantum: expected integer in range [1,2^31-1].\n");
    exit(-1);
}

/** Queue management functions **/

/* creates a queue node (to be attached to a queue) to hold a process */
process_node *createProcessNode(process *p) {
    process_node *node = (process_node*) malloc(sizeof(process_node));
    if (node == NULL) error("out of memory");
    node->data = p;
    node->next = NULL;
    return node;
}

/* performs basic initialization on the process queue `q' */
void initializeProcessQueue(process_queue *q) {
    q->front = q->back = NULL;
    q->size = 0;
}

/* enqueues a process `p' at the back of the process queue pointed to by `q'.
   warning: Process pointer `p' must not point to a temporary memory location.
            It must be accessible for as long as the queue is. */

void enqueueProcess(process_queue *q, process *p) {
    process_node *node = createProcessNode(p);

    if (q->front == NULL) {
        assert(q->back == NULL);
        q->front = q->back = node;
    } else {
        assert(q->back != NULL);
        q->back->next = node;
        q->back = node;
    }
    q->size++;
}

/* dequeues a process from the front of the process queue pointed to by `q' */
void dequeueProcess(process_queue *q) {
    process_node *deleted = q->front;

    assert(q->size > 0);
    if (q->size == 1) {
        q->front = NULL;
        q->back = NULL;
    } else {
        assert(q->front->next != NULL);
        q->front = q->front->next;
    }
    free(deleted);
    q->size--;
}

/** Input/output functions **/

/* reads a line from stdin and returns it (in dynamically allocated space) */
char *readLine(void) {
    char *prefix = (char*) calloc(1, 1);
    if (prefix == NULL) error("out of memory");
    return readLineHelper(prefix, 16);
}

/* recursive helper for readLine.
   reads a line and returns it (in dynamically allocated space).
   note: prefix should be dynamically allocated, and will be freed.
         n > strlen(prefix) is the expected size of the string.
           memory allocated for the string will be doubled in size
           every time the line does not fit entirely within it.
*/
char *readLineHelper(char *prefix, int n) {
    int prefixlen = strlen(prefix);
    char *result = (char*) calloc(n, 1);
    if (result == NULL) error("out of memory");
    
    assert(prefixlen < n);
    memcpy(result, prefix, prefixlen);
    
    if (fgets(result+prefixlen, n-prefixlen, stdin) == NULL) return NULL;
    if (strchr(result, '\n') == NULL) return readLineHelper(result, 2*n);
    
    free(prefix);
    return result;
}

/* reads a non-negative integer from *buf and returns it.
   before the result is returned, *buf is incremented to point to the
       first location after the integer.
   if there is no such integer, or it does not fit in a 32-bit signed int,
       return value is -1. */
int readInt(char **buf) {
    int result = 0;
    
    /* skip whitespace */
    while (isspace(**buf)) (*buf)++;
    
    /* check if buffer is empty */
    if (**buf == '\0') return -1;
    
    /* accumulate integer value */
    while (**buf && !isspace(**buf)) {
        int new_result = 0;
    
        /* check for invalid characters */
        if (**buf < '0' || **buf > '9') return -1;
        
        /* accumulate digit (checking for overflow) and advance pointer */
        new_result = result * 10 + (**buf - '0');
        if (new_result < result) return -1;
        result = new_result;
        (*buf)++;
    }
    //printf("READINT %d\n" , result);
    return result;
}

/* same as readInt, but expects an int enclosed in braces ().
   returns -1 if no suitable int exists, according to rules of readInt,
       OR if the next int is not enclosed in 1 set of braces. */
int readBracedInt(char **buf) {
    int result = 0;
    
    /* skip whitespace */
    while (isspace(**buf)) (*buf)++;
    
    /* check if buffer is empty */
    if (**buf == '\0') return -1;

    /* skip open brace */
    if (**buf == '(') (*buf)++;
    else return -1;
    
    /* accumulate integer value */
    while (**buf && !isspace(**buf) && **buf != ')') {
        int new_result = 0;
    
        /* check for invalid characters */
        if (**buf < '0' || **buf > '9') return -1;
        
        /* accumulate digit (checking for overflow) and advance pointer */
        new_result = result * 10 + (**buf - '0');
        if (new_result < result) return -1;
        result = new_result;
        (*buf)++;
    }
    
    if (**buf == ')') (*buf)++;
    else return -1;
    
    return result;
}

/* returns non-zero value iff. any characters in s are whitespace */
int empty(char *s) {
    /* skip white space and check first non-whitespace character */
    while (isspace(*s)) s++;
    return (*s == '\0');
}

/* reads all information pertaining to a single process from standard input,
   and stores the result into `dest'.  the return value is 1 if there are
   more process(es) to be read in the input, and 0 otherwise.
   (also returns COMMENT_LINE in event of a comment, signified by a leading
    COMMENT_CHAR character, being read.) */
int readProcess(process *dest) {
    //int i;
    int pid = -1;
    int arrivalTime = 0;
    int firstBurst = 0;
    int cpuBurstLength = 0;
    int ioBurstLength = 0;
    
    char *line = readLine();
    char *ptr = line;
    
    if (line == NULL) return 0;
    
    /* ignore blank lines and handle 'comments' in input file (added feature) */
    if (empty(line) || line[0] == COMMENT_CHAR) return COMMENT_LINE;
    
    /* retrieve pid, arrival time and first burst */
    if ((pid = readInt(&ptr)) < 0) error_malformed_input_line(line);
    if ((arrivalTime = readInt(&ptr)) < 0) error_malformed_input_line(line);
    if ((firstBurst = readInt(&ptr)) < 0) error_malformed_input_line(line);
    //printf("----------------------------\n");
    /* save these parameters to the process */
    dest->pid = pid;
    dest->arrivalTime = arrivalTime;
    dest->bursts[0].length = firstBurst;
    dest->bursts[0].step = 0;
    dest->numberOfBursts = 1;
    
    /* read in the rest of the io and cpu bursts in pairs ([IO, CPU], ...) */
    while (!empty(ptr)) {
        
        /* get and save I/O burst, erroring out if it cannot be read */
        if ((ioBurstLength = readBracedInt(&ptr)) == -1) {
            error_malformed_input_line(line);
        }
        //printf("IOBracedInt:%d\n" , ioBurstLength);
        //printf("---------------------------------------\n");
        dest->bursts[dest->numberOfBursts].step = 0;
        dest->bursts[dest->numberOfBursts].length = ioBurstLength;
        dest->numberOfBursts++; // increment gives CPU burst
        
        /* get and save CPU burst */
        if ((cpuBurstLength = readInt(&ptr)) == -1) {
            error_malformed_input_line(line);
        }
        //printf("CPU BURST %d\n" , cpuBurstLength);
        /* check for too many bursts first!
          (here is where we will see the 1001th burst if it occurs) */
        if (dest->numberOfBursts == MAX_BURSTS) {
            error_too_many_bursts(dest->pid);
        }
        dest->bursts[dest->numberOfBursts].step = 0;
        dest->bursts[dest->numberOfBursts].length = cpuBurstLength;
        dest->numberOfBursts++;
    }
    free(line);
    
    return 1;
}

/* comparator for process structs that orders them by earliest arrival time,
   breaking ties by smallest pid.
   That is, process a is less than process b iff:
        a arrives before b, OR
        they arrive at the same time and pid(a) < pid(b). */
int compareByArrival(const void *aa, const void *bb) {
    process *a = (process*) aa;
    process *b = (process*) bb;
    if (a->arrivalTime < b->arrivalTime) return -1;
    if (a->arrivalTime > b->arrivalTime) return 1;
    return 0;
}