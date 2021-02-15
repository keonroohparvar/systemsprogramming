#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>



void readDir(DIR *directory);
void nullFunction(int a);
void meanFunction(int a);
void meanFunction2(int a);
void meanFunction3(int a);

int main()
{
    //Redirection of Signals in parent to a garbage function
    signal(SIGTSTP, nullFunction);
    signal(SIGINT, nullFunction);
    signal(SIGQUIT, nullFunction);
    
    


    //Actual Program
    int parentPid = getpid();
    int childPid;
    int *childStatus = (int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int childCount;
    printf("The Parent's PID: %d\n", parentPid);
    while (1)
    {
        if (fork() == 0)
        {
            signal(SIGTSTP, meanFunction);
            signal(SIGINT, meanFunction2);
            signal(SIGQUIT, meanFunction3);
            childPid = getpid();
            childCount = -1;
            while (1)
            {
                //Child Loop
                printf("\n-----------------------------------\n");
                printf("The Child's PID: %d\n", childPid);
                char cwd[1000];
                while (1)
                {
                    childCount = (childCount + 1) % 10;
                    sleep(1);
                    if (childCount == 0)
                    {
                        time_t T= time(NULL);
                        struct tm tm=*localtime(&T);//tm.tm_hour,tm.tm_min
                        printf("\n-----------------------------------\n");
                        printf("Time: %d:%d\n", tm.tm_hour, tm.tm_min);
                        getcwd(cwd, 1000);
                        printf("Current directory: %s\n", cwd);
                    }
                }
                
                return 1;
            }
        }
        wait(0);
        printf("Child process has ended. Reforking.\n");
    }

    return 1;
}

void nullFunction(int a) {
    a = a + 1;
}

void meanFunction(int a)
{
    printf("YOU CAN'T DESTROY ME HUMAN\n");
}

void meanFunction2(int a)
{
    printf("Installing malware - PROGRESS : 0/100\n");
    printf("Installing malware - PROGRESS : 100/100\n");
    //To-do: write function to install malware (joke please don't hate me)
    printf("Process done. Thank you for downloading from piracy.net!\n\n");
}

void meanFunction3(int a)
{
    printf("NOPE\n");
}