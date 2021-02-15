#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

/* Function Headers */
void handleRequest(int index);
void handleRelease(int index);
void sendRequest();
void sendRelease();
void entryProtocol();
void exitProtocol();

/* Maekawa's algorithm's initializations */
int numProcesses;
int *quorum; /* Quorum is shared memory that points to an array of each process' PID */
int *replyReceived; /* Shared array that at each index: 0 = reply not received, 1 = reply recieved */
int doneWithCritSection;

/* if thisProgram = 0, thisProgram is parent; otherwise, it is child  
 * if otherProgram = 1, thisProgram is parent, and vice versa. This is
 * needed for quorom to know where to send signals. */
int thisProgram;
int otherProgram;
int *waitTime;


int main() 
{
    doneWithCritSection = 1;
    numProcesses = 2; /* Update this to the number of processes - 2 in this case. */
    quorum = (int *)mmap(NULL, (int)(sizeof(int) * numProcesses), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    replyReceived = (int *)mmap(NULL, (int)(sizeof(int) * numProcesses), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    quorum[0] = getpid(); /* Parent's PID is quorum[0] */
    thisProgram = 0;
    otherProgram = 1;
    waitTime = (int *)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *(waitTime) = 2;
    
    printf("Parent PID: %d\n", quorum[0]);
    char parentsArr[256];

    signal(SIGUSR1, handleRequest); /* USR1 signal is for Request */
    signal(SIGUSR2, handleRelease); /* USR2 signal is for the release */

    char *criticalSec = (char *)mmap(NULL, 1000, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    
    if (fork() == 0)
    {
        thisProgram = 1;
        otherProgram = 0;
        int childPID = getpid();
        quorum[1] = childPID;
        char sentence1[] = "Hello, my name is jared, I am 19, and I never learned how to read. :( #sadboihours\n";
        char sentence2[] = "Why do cows not have toes? ... Because they lactose! Hahahah, I am so funny! I want friends :(\n";
        printf("Child pid: %d\n", childPID);
        wait(waitTime);
        while(1)
        {
            /* Child's infite loop */
            /* To not use mutex, comment out entryProtocol(1) and exitProtocol(1) */
            entryProtocol();
            wait(waitTime);
            strcpy(criticalSec, sentence1);
            exitProtocol();
            wait(waitTime);
            entryProtocol();
            wait(waitTime);
            strcpy(criticalSec, sentence2);
            exitProtocol();
            wait(waitTime);
        }
    }
    while(1)
    {
        /* Parent's infinite loop */
        /* To not use mutex, comment out entryProtocol() and exitProtocol() */
        while(quorum[1] == 0)
        {/* waits for kid to update quorum*/}
        entryProtocol();
        strcpy(parentsArr, criticalSec);
        printf("%s", parentsArr);
        exitProtocol();
    }
    return 1;
}

void entryProtocol()
{
    doneWithCritSection = 0;
    sendRequest();
}

void exitProtocol()
{
    doneWithCritSection = 1;
    sendRelease();
    while (replyReceived[otherProgram] == 1)
    {
        /* wait until other program is done*/
    }
    replyReceived[thisProgram] = 0;
}

void sendRequest()
{
    kill(quorum[otherProgram], SIGUSR1);
    while(replyReceived[thisProgram] == 0)
    { 
        /* Stuck in while loop until received responses from other program */ 
    }
}

void sendReply()
{
    replyReceived[otherProgram] = 1;   
}

void sendRelease()
{
    /* Obviously this is a little silly with 2 programs, but this is how
     * it is done with multiple programs and I am hard-coding it to work when 
     * there are only two procresses. */
    handleRelease(4); 
}

void handleRequest(int a) 
{
    if (doneWithCritSection == 1)
    {
        replyReceived[otherProgram] = 1;
    }
    else
        replyReceived[otherProgram] = 0;
}

void handleRelease(int a)
{
    sendReply();
    replyReceived[thisProgram] = 0;
}