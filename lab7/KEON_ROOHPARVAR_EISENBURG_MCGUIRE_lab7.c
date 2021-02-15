#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

enum states {IDLE, WAITING, ACTIVE};
int numProcesses;
enum states *flags;
int turn;

void init(enum states flags[]);
int entryProtocol(int index);
int exitProtocol(int index);

int main()
{
    /* Intializations for Eisenburg & McGuire's algorithm */
    numProcesses = 2; /* Can be set to the desired number of processes instead of 2 */
    printf("Size of states: %ld\n", sizeof(enum states));
    int sizeForFlags = (int)(numProcesses * (sizeof(enum states)));
    printf("sizeforflags: %d\n", sizeForFlags);
    flags = (enum states *) mmap(NULL, sizeForFlags, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); 
    
    printf("Flags[0]: %d\n", flags[0]);
    init(flags);

    /* Inializations for program */
    int parentPID = getpid();
    printf("Parent ID: %d\n", parentPID);
    char *criticalSec = (char *)mmap(NULL, 1000, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    if (fork() == 0)
    {
        /* Child process */
        int childPID = getpid();
        char sentence1[] = "Hello, my name is jared, I am 19, and I never learned how to read. :( #sadboihours\n";
        char sentence2[] = "Why do cows not have toes? ... Because they lactose! Hahahah, I am so funny! I want friends :(\n";
        printf("Child pid: %d\n", childPID);
        while(1)
        {
            /* Child's infite loop */
            /* To not use mutex, comment out entryProtocol(1) and exitProtocol(1) */
            entryProtocol(1);
            strcpy(criticalSec, sentence1);
            exitProtocol(1);
            entryProtocol(1);
            strcpy(criticalSec, sentence2);
            exitProtocol(1);
        }
        return 1; 
    }
    char parentsArr[256];
    while(1)
    {
        /* Parent's infinite loop */
        /* To not use mutex, comment out entryProtocol(0) and exitProtocol(0) */
        entryProtocol(0);
        strcpy(parentsArr, criticalSec);
        printf("%s\n", parentsArr);
        exitProtocol(0);
    }
    return 1;
}

int entryProtocol(int index)
{   int i;
    do {
        /* Current process tries to enter the critical block */
        flags[index] = WAITING;

        /* Scan processes starting at turn up to ourselves */
        i = turn;
        while (i != index)
        {
            if (flags[i] != IDLE) { i = index; }
            else { i = (i + 1) % numProcesses; } 
        }

        /* Setting our program to enter the critical section */
        flags[index] = ACTIVE;

        /* Updates i to the FIRST active program outside of the current program */
        i = 0;
        while ((i < numProcesses) && (i == index || flags[i] != ACTIVE))
            i = i + 1;

    } while ((i < numProcesses) && (turn != index && flags[turn] != IDLE));

    turn = index;
}

int exitProtocol(int index)
{
    int i = index + 1;

    /* Updates i to first process besides ours that is NOT idle.
     * If this does not exist, it updates to ourselves as we are not idle. */
    while (flags[i] == IDLE)
        i = (i + 1) % numProcesses;
    
    turn = i;

    flags[index] = IDLE;
}

void init(enum states flags[])
{
    /* Initializes all flags to IDLE and sets turn to 0*/
    int index;
    for (index = 0; index < numProcesses; index++)
    {
        flags[index] = IDLE;
    }
    turn = 0;
}