#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/sysmacros.h>

#define KBLU "\x1B[34m" /* This is the blue color */
#define KNRM  "\x1B[0m" /* This is the normal color */

int statHandler(char fileName[]);
void signalHandler(int a);
void junkSignalHandler(int a);

int main()
{   
    /* Signal Handlers */
    signal(SIGTSTP, signalHandler);

    /* Initializations */
    char inputText[256];
    int parentPID = getpid();
    int *childPID = (int *) mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *childExitStatus = (int *) mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    char currentDir[256];
    strcpy(inputText, "null"); /* This is to initialize it to SOMETHING */
    
    
    printf("Parent PID: %d\n", parentPID);

    /* Parent process */ 
    while (*childExitStatus != 1)
    {
        /* Child process */ 
        if (fork() == 0)
        {
            /* Signal Handlers and initializations */
            signal(SIGTSTP, junkSignalHandler);
            DIR *dir;
            struct dirent *dp;
            getcwd(currentDir, sizeof(currentDir));
            int changeDirStatus;

            /* Printing the Child's PID*/
            *childPID = getpid();
            *childExitStatus = 0;
            printf("Child PID: %d\n", *childPID);
            
            /* Gathering the first input from User*/
            printf("%skeon's_FUN_CLI.%s%s$ ",KBLU, currentDir, KNRM);
            fflush(stdin);
            scanf("%s", inputText);

            while (strcmp(inputText, "q") != 0)
            {
                /* Code for printing the files in current directory */
                if (strcmp(inputText, "list") == 0)
                {
                    if ((dir = opendir(currentDir)) == NULL) 
                    {
                        printf("Could not open current directory.\n");
                    }
                    else
                    {
                        printf("Current Directory: %s%s%s\n", KBLU, currentDir, KNRM);
                        printf("Files in this directory include:\n");
                        while ((dp = readdir(dir)) != NULL)
                        {
                            printf(" -> %s\n", dp->d_name);
                        }
                        printf("\n");
                    }
                    closedir(dir);
                }
                /* Going one directory higher */
                else if (strcmp(inputText, "..") == 0)
                {
                    getcwd(currentDir, sizeof(currentDir));
                    changeDirStatus = chdir("..");
                    getcwd(currentDir, sizeof(currentDir));
                    if (changeDirStatus == -1)
                    {
                        printf("There was an error changing directories. Please confirm that the entered path is a directory from the current working directory.\n");
                    }
                    else
                    {
                        printf("Changed current directory to: %s\n", currentDir);
                    }
                    
                }
                /* Going into a directory*/
                else if (inputText[0] == '/')
                {
                    char newDir[256];
                    strcpy(newDir, ".");
                    strcat(newDir, inputText);
                    changeDirStatus = chdir(newDir);
                    getcwd(currentDir, sizeof(currentDir));
                    if (changeDirStatus == -1)
                    {
                        printf("There was an error changing directories. Please confirm that the entered path is a directory from the current working directory.\n");
                    }
                    else
                    {
                        printf("Changed current directory to: %s\n", currentDir);
                    }
                }

                /* Code for printing the status of a file. You don't need to check the condition
                * if(strcmp(inputText, "q") == 0) because the program will break out of the 
                * while loop if inputText is q AT THE START, so it is assumed that the inputText 
                * is not q if the program counter is here. 
                */
                else 
                {
                    statHandler(inputText);
                }

                /* Ask for input again */
                printf("%skeon's_FUN_CLI.%s%s$ ",KBLU, currentDir, KNRM);
                fflush(stdin);
                scanf("%s", inputText);
            }
            *childExitStatus = 1;
            return 1;
        }
        wait(0);
        if (*childExitStatus != 1)
        {
            printf("\nChild process exited - Rebooting in 10 seconds.\n");
            sleep(10);
            printf("REBOOTED\n\n");
        }
    }
    printf("\nExiting...\n");
    return 1;    
}

void signalHandler(int a)
{
    printf("\nCannot suspend the program - IT CREATES ZOMBIES :(\n");
    printf("Try control+c :)\n");
}

void junkSignalHandler(int a)
{
    ;
}

int statHandler(char fileName[])
{
    struct stat sb; 
    int openFileStatus = stat(fileName, &sb);
    if (openFileStatus == -1)
    {
        printf("ERROR - There was no file found with that name. Try again.\n");
        return -1;
    }
    printf("ID of containing device:  [%lx,%lx]\n", (long) major(sb.st_dev), (long) minor(sb.st_dev));
    printf("File type:                ");
    switch (sb.st_mode & S_IFMT) 
    {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }

    printf("I-node number:            %ld\n", (long) sb.st_ino);
    printf("Mode:                     %lo (octal)\n", (unsigned long) sb.st_mode);
    printf("Link count:               %ld\n", (long) sb.st_nlink);
    printf("Ownership:                UID=%ld   GID=%ld\n", (long) sb.st_uid, (long) sb.st_gid);
    printf("Preferred I/O block size: %ld bytes\n",(long) sb.st_blksize);
    printf("File size:                %lld bytes\n",(long long) sb.st_size);
    printf("Blocks allocated:         %lld\n", (long long) sb.st_blocks);
    printf("Last status change:       %s", ctime(&sb.st_ctime));
    printf("Last file access:         %s", ctime(&sb.st_atime));
    printf("Last file modification:   %s", ctime(&sb.st_mtime));
}