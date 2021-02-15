#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>



int *childPids;

int main(int argc, char *argv[20] ) 
{
    if (argc != 3)
    {
        printf("Error - Please include the correct number of arguments.\n");
        return -1;
    }

    /* Initialize command line arguments */
    char progName[256];
    strcpy(progName, argv[1]);
    int numProcesses = atoi(argv[2]);
    if (numProcesses == 0)
    {
        printf("Error - please format arguments correctly.\n");
        return -1;
    }
    if (numProcesses != 1 && numProcesses != 2 && numProcesses != 4 && numProcesses != 8 && numProcesses != 16)
    {
        printf("Error - Please restrict the number of processes to one of the following:\n1, 2, 4, 8, 16\n");
        return -1;
    }
    
    /* Init shared memory "readyList" which will be used for synching child processes */
    int fd = shm_open("readyList", O_RDWR|O_CREAT, 0777);
    ftruncate(fd, 16*sizeof(int));


    /* Forking and execv calls */
    int thisPid;
    childPids = (int *) mmap(NULL, 16*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < numProcesses; i++)
    {
        if (fork() == 0)
        {
            /* This is to eliminate zombies at the end. */
            thisPid = getpid();
            childPids[i] = thisPid;

            /* Format arguments and call program. */
            char n[50], thisProcess[50];
            sprintf(n, "%d", numProcesses);
            sprintf(thisProcess, "%d", i);
            char *process2Args[] = {progName, thisProcess, n, NULL};
            execv(progName, process2Args);
            return 1;
        }
    }

    for (int i=0; i<numProcesses; i++)
    {
        while(childPids[i] == 0) 
        { 
            /* Waits for kid to update its pid */
        }

        /* Gets rid of zombies. */
        wait(&(childPids[i]));
    }

    munmap(childPids, 16*sizeof(int));

    return 1;

}
