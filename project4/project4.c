#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdlib.h>
int fd[2];
int *childpids;
char *childstatus[10];

#define KBLU "\x1B[34m" /* This is the blue color */
#define KNRM  "\x1B[0m" /* This is the normal color */
#define KRED "\x1B[32m" /* This is the green color */


/* childReportLength is a mmap int that will be used for the parent program while 
 * thisReportLength is a local variable per program that will be used for each 
 * child to update childReportLength.
 * thisReport is the actual text of the report that will get written to the pipe.
*/
int *childReportLength;
int thisReportLength = 0;
char thisReport[256]; 

#define _GNU_SOURCE /* This is done for checking if a file is a directory */

//////////////////////////////////////////////////////////////////////
// get argument function
//
// return true if ok and false if error or no further argument
// line .. the command line
// argn .. the to extracted argument beginning with 0
// result .. should be a static char array of 1000 bytes
//
// it respects quotation marks!!
// its also not much tested
// 
// 
//////////////////////////////////////////////////////////////////////
int get_argument(char* line, int argn, char* result)
{
	//firstly: remove all spaces at the front
	char temp[1000];
	int start_space = 1;
	for (int i = 0, u = 0; i <= strlen(line); i++)
		if (line[i] == ' ' && start_space == 1) continue;
		else 
        {
			temp[u++] = line[i]; 
			start_space = 0;
        }
	//now remove an double or tripple spaces
	char temp2[1000];
	int space_on = 0;
	for (int i = 0, u = 0; i <= strlen(temp); i++)
    {
		if (space_on == 1 && temp[i] == ' ') continue;
		temp2[u++] = temp[i];
		if (temp[i] == ' ') space_on = 1;
		else space_on = 0;	
    }
	//finally extract the arguments
	int start, end;
	start = end = 0;
	int count = 0;
	int quote = 0;
	for (int i = 0; i <= strlen(temp2); i++)
		if (temp2[i] == '\"') quote = !quote;
		else if (quote == 0 &&(temp2[i] == ' ' || temp2[i] == 0))
        {
			end = i;
			if (argn == count)
            {
				int length = end - start;
				strncpy(result, temp2 + start, length);
				result[length] = 0;
				return 1;
            }
			start = end + 1;
			count++;
        }
	return 0;
}

void printHelp()
{
    printf("\n----- Commands ------\n\n");
    printf("%sfind <fileName> -flag1(Optional) -flag2(Optional)%s : Search current directory for file fileName.\n", KBLU, KNRM);
    printf("Flags:\n     %s-s%s -> Searches subdirectories as well.\n     %s-f:fileEnding%s -> Limits search to files only if the end of their filename matches fileEnding\n\n", KBLU, KNRM, KBLU, KNRM);

    printf("%sfind \"searchText\" -flag1(Optional) -flag2(Optional)%s : Search files in current directory for string \"searchText\"\n", KBLU, KNRM);
    printf("Flags:\n     %s-s%s -> Searches subdirectories as well.\n     %s-f:fileEnding%s -> Limits search to files only if the end of their filename matches fileEnding\n\n", KBLU, KNRM, KBLU, KNRM);

    printf("%slist%s : lists all current child processes and what they are doing.\n\n", KBLU, KNRM);
    printf("%skill <num>%s : kills a child process specified by its number (1-10)\n\n", KBLU, KNRM);
    printf("%squit%s or %sq%s : kills all child processes and exits the program.\n\n", KBLU, KNRM, KBLU, KNRM);
    printf("%shelp%s : views this menu.\n", KBLU, KNRM);
    printf("\n");
}

void add_null_term(char *txt)
{
    for(int i=0;i<100;i++)
        if(txt[i]=='\n')    
        {
            txt[i+1]=0;break;
        }
}

int overridemode=0;

void myfct(int y)
{
    dup2(fd[0],STDIN_FILENO); //Overwrite userinput
    overridemode=1;
}

int findChild()
{
    /* This function will return the first available child, -1 if none exist */
    for(int i=0;i<10;i++) 
        if(childpids[i]==0) 
        {
            childpids[i]=getpid();
            return i;
        }
    return -1;
}

void updateChildStatus(int childNum, char *statusText)
{
    strcpy(childstatus[childNum], statusText);
}

int fileFound = 0;
void searchDir(char *fileName, char cwd[256], int sFlag)
{
    /* Recursive Function that searches Directories for a file.
     * if s = 0, -s is not set. if s = 1, -s is set.
    */
    char foundText[1000];
    DIR *dir;
    struct dirent *dp;
    int correctFile = 0; 
    
    dir = opendir(cwd);

    while ((dp = readdir(dir)) != NULL)
    {
        /* Iterares through each of the directories files.
         * After checking criteria, this while loop will update correctFile to 1,
         * signaling that the correct file was found (mainly used for f flag).
         */
        //printf("FILE: %s in dir: %s\n", dp->d_name, cwd);
        if (sFlag == 1 && dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)
        {
            /* Recursive call for subdirectories*/
            char newDir[256];
            strcpy(newDir, cwd);
            strcat(newDir, "/");
            strcat(newDir, dp->d_name);
            searchDir(fileName, newDir, 1);
        }
        else if (strncmp(dp->d_name, fileName, strlen(fileName)) == 0)
        {
            fileFound = 1; /* Signals that the correct file is found */
            strcpy(foundText, fileName);
            strcat(foundText, " found in directory ");
            strcat(foundText, cwd);
            strcat(foundText, "\n");
            strcat(thisReport, foundText);
        }
    }
    closedir(dir);
}

int textFound = 0;
int searchFile(char *filePath, char *text)
{
    /* Searches a file at the filePath for a certain text.
     * Returns 1 if found, returns 0 if not found, returns -1 if error.
     */
    int textLength = strlen(text);

    char foundText[1000];
    char cwd[256];
    FILE *file;
    if (!(file = fopen(filePath, "rb")))
    {
        printf("Error opening file at %s\n", filePath);
        return -1;
    }

    /* Initialize the current char and iterate in file */
    char currentChar;
    int correctChars = 0;
    while (fread(&currentChar, 1, 1, file) == 1)
    {
        if (correctChars == textLength)
        {
            /* Found text in the file. */
            fclose(file);
            textFound = 1;
            return 1;
        }
        else if (correctChars != 0)
        {
            /* Entered this if you starting reading chars that match text */
            if (currentChar == text[correctChars])
                correctChars++;
            else
                correctChars = 0;
        }
        else if (currentChar == text[0])
        {
            correctChars = 1;
        }
    }
    fclose(file);
    return 0;
}

void searchText(char *text, char cwd[256], int sFlag, char *fFlag)
{
    /* Recursive Function that searches files for a specified text.
     * if s = 0, -s is not set. if s = 1, -s is set.
     * if fFlag = '\0', then -f is not set. Otherwise, fFlag is set.
    */
    char foundText[1000];
    DIR *dir;
    struct dirent *dp;
    
    dir = opendir(cwd);
    textFound = 0;
    int thisFileFound = 0;
    char filePath[256];
    while ((dp = readdir(dir)) != NULL)
    {
        /* Iterares through each of the directories files.
         * After checking criteria, it will search each file that matches
         * certain criteria for the given string "text".
         */
        thisFileFound = 0;
        strcpy(filePath, cwd);
        strcat(filePath, "/");
        strcat(filePath, dp->d_name);
        if (sFlag == 1 && dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0)
        {
            /* Recursive call for subdirectories*/
            char newDir[256];
            strcpy(newDir, cwd);
            strcat(newDir, "/");
            strcat(newDir, dp->d_name);
            searchText(text, newDir, 1, fFlag);
        }
        else if (dp->d_type == DT_DIR)
        {
            /* NOTE THIS DOES NOTHING.
             * Program counter enters this if file is a directory and -s is NOT set, 
             * so that searchFile() is not called on a directory.
            */
        }
        else if (*(fFlag) != '\0')
        {
            int lenOfFile = strlen(dp->d_name);
            int lenOfFFlag = 0;
            while (fFlag[lenOfFFlag] != 0 && fFlag[lenOfFFlag] != '\n')
                lenOfFFlag++;

            if (lenOfFFlag <= lenOfFile) 
            { 
                char *lastChars = &(dp->d_name[lenOfFile - lenOfFFlag]);
                if (strncmp(lastChars, fFlag, lenOfFFlag) == 0)
                {
                    /* Check the file here because its end matches fFlag's end */
                    thisFileFound = searchFile(filePath, text);
                }
            }
        }
        else
        {
            /* If no fFlag or sFlag */
            thisFileFound = searchFile(filePath, text);
        }

        if (thisFileFound == 1)
        {
            /* If text found, function writes to pipe. */
            char textFoundMessage[256];
            strcpy(textFoundMessage, "The text '");
            strcat(textFoundMessage, text);
            strcat(textFoundMessage, "' found in file ");
            strcat(textFoundMessage, dp->d_name);
            strcat(textFoundMessage, " at ");
            strcat(textFoundMessage, filePath);
            strcat(textFoundMessage, "\n");
            strcat(thisReport, textFoundMessage);
        }
    }
    closedir(dir);
}

int main()
{
    /* Initialize childpids - holds all of child processes' pids */
    childpids = (int *)mmap(0,sizeof(int)*10,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    for(int i=0;i<10;i++) 
        childpids[i]=0;

    /* Initialize childstatus - holds info on what each child is doing */
    for(int i=0;i<10;i++) 
        childstatus[i]= (char *) mmap(0,sizeof(char)*256,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    
    /* Initialize childreport - used towards reading the correct # of bytes per 
     * each report from a child process 
     */        
    childReportLength = (int *) mmap(0,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

    signal(SIGUSR1,myfct);
    char input[1000];
    int parentPid = getpid();
    pipe(fd);
    int save_stdin = dup(STDIN_FILENO);
    printf("\nWelcome to Keon's search engine! Type \"help\" to look at available commands.\n");
    while (1)
    {
        thisReport[0] = '\0';
        printf("%sfindstuff$%s ", KRED, KNRM);
        fflush(0);
        dup2(save_stdin,STDIN_FILENO);
        overridemode=0;

        read(STDIN_FILENO,input,1000);
        char firstArg[1000];
        get_argument(input, 0, firstArg);
        if(overridemode==0)
            add_null_term(input); //to get a NULL at the end of the string in case of a user input

        else if (overridemode == 1)
        {
            printf("%s", input);
            fflush(0);
        }
        if (strncmp(firstArg,"find",4) == 0)
        {

            if (fork() == 0) 
            {
                //search for an empty spot in the child list
                int errorExists = 0; /* This is updated to 1 when there is an error */
                int kidnum = findChild();
                char kidNumChar[50];
                if (kidnum == -1)
                {
                    strcat(thisReport, "ERROR - Too many child processes active. Please wait for one to close before continuing your operation.\n");
                    errorExists = 1;
                }

                if (errorExists == 0)
                {
                strcat(thisReport, "\n\n - CHILD ");  
                sprintf(kidNumChar, "%d", kidnum);    
                strcat(thisReport, kidNumChar);
                strcat(thisReport, " REPORT - \n");    
                }

                close(fd[0]); //close read
                char currentDir[256];
                getcwd(currentDir, sizeof(currentDir));
                fileFound = 0;
                textFound = 0;

                /* Format flags for text search */
                char secondArr[256], thirdArr[256], fourthArr[256];
                int secondArrStatus = get_argument(input, 1, secondArr);
                if (secondArrStatus == 0 && errorExists == 0)
                {
                    thisReport[0] = '\0';
                    strcat(thisReport, "\n\nError - incorrectly formatted arguments. See help for more information.");
                    childpids[kidnum] = 0;
                    errorExists = 1;
                }
                if (secondArr[0] == '"' && errorExists == 0)
                {
                    /* Extract the search text without the " characters */
                    char textToFind[256];
                    int i;
                    for (i = 1; secondArr[i] != '"'; i++)
                    {
                        textToFind[i - 1] = secondArr[i];
                        if (i == 256)
                        {
                            thisReport[0] = '\0';
                            strcat(thisReport, "\n\nError - Incorrectly formatted arguments or search string too long.");
                            childpids[kidnum] = 0;
                            errorExists = 1;
                            break;
                        }
                    }
                    textToFind[i-1] = '\0';
                    
                    /* Update child status */
                    char childStatusText[256];
                    strcpy(childStatusText, "Searching for ");
                    strcat(childStatusText, textToFind);
                    strcat(childStatusText, " text.");
                    updateChildStatus(kidnum, childStatusText);
                    

                    /* Extract sFlag and fFlag from command line */
                    int sFlag = 0;
                    char fFlag[256];
                    int thirdExists = get_argument(input, 2, thirdArr);
                    int fourthExists = get_argument(input, 3, fourthArr);
                    if (fourthExists == 1 && errorExists == 0)
                    {
                        /* If fourth argument exists, then so does third */
                        if (thirdArr[1] == 's' || fourthArr[1] == 's')
                            sFlag = 1;
                        if (thirdArr[1] == 'f')
                        {
                            int i;
                            int j = 0;
                            for (i = 3; thirdArr[i] != 0; i++)
                            {
                                fFlag[j] = thirdArr[i];
                                j = j + 1;
                            }
                            fFlag[j] = 0;
                        }
                        else if (fourthArr[1] == 'f')
                        {
                            int i;
                            int j = 0;
                            for (i = 3; fourthArr[i] != 0; i++)
                            {
                                fFlag[j] = fourthArr[i];
                                j = j + 1;
                            }
                            fFlag[j] = 0;
                        }
                        searchText(textToFind, currentDir, sFlag, fFlag);
                    }
                    else if (thirdExists == 1 && errorExists == 0)
                    {
                        /* If there are 3 arguments */

                        if (thirdArr[1] == 's')
                            sFlag = 1;
                        if (thirdArr[1] == 'f')
                        {
                            int i;
                            int j = 0;
                            for (i = 3; thirdArr[i] != 0; i++)
                            {
                                fFlag[j] = thirdArr[i];
                                j = j + 1;
                            }
                            fFlag[j] = 0;
                        }
                        searchText(textToFind, currentDir, sFlag, fFlag);
                    }

                    else if (errorExists == 0)
                    {
                        /* Search with no flags */
                        searchText(textToFind, currentDir, 0, "\0");
                    }

                    if (textFound == 0 && errorExists == 0)
                    {
                        char textNotFoundText[256];
                        textNotFoundText[0] = '\0';
                        strcat(textNotFoundText, "The text string '");
                        strcat(textNotFoundText, textToFind);
                        strcat(textNotFoundText, "' was not found.\n");
                        strcat(textNotFoundText, "\0");
                        strcat(thisReport, textNotFoundText);
                    }
                }
                else
                {   
                    /* Searching for files. */
                    int secondArrExists = get_argument(input, 1, secondArr);
                    int thirdArrExists = get_argument(input, 2, thirdArr);
                    int fourthArrExists = get_argument(input, 3, fourthArr);
                    if (fourthArrExists == 1 && errorExists == 0)
                    {
                        thisReport[0] = '\0';
                        strcat(thisReport, "\n\nError - too many arguments for find <filename>. Type help for information.");
                        errorExists = 1;
                    }


                    int sFlag = 0;

                    if (secondArrExists == 0 && errorExists == 0)
                    {
                        thisReport[0] = '\0';
                        strcat(thisReport, "\n\nError - Please include the filename with the find command.");
                        errorExists = 1;
                    }

                    /* Update child status */
                    char childStatusText[256];
                    strcpy(childStatusText, "Searching for ");
                    strcat(childStatusText, secondArr);
                    strcat(childStatusText, " text.");
                    updateChildStatus(kidnum, childStatusText);

                    if (thirdArrExists == 0 && errorExists == 0)
                    {
                        /* Removes last null character on second argument */
                        secondArr[strlen(secondArr) - 1] = '\0';
                    }

                    if (thirdArr[0] == '-' && thirdArr[1] == 's' && errorExists == 0)
                    {
                        searchDir(secondArr, currentDir, 1); 
                    }   
                    else if(errorExists == 0)
                    {
                        searchDir(secondArr, currentDir, 0); 
                    }
                    if (fileFound == 0 && errorExists == 0)
                    {
                        char fileNotFoundText[256];
                        strcpy(fileNotFoundText, "This file was not found. Please confirm this is the correct file name.\n");
                        strcat(thisReport, fileNotFoundText);
                    }  
                }
                
                fflush(0);
                strcat(thisReport, "\n\n");
                write(fd[1], thisReport, strlen(thisReport) + 1);
                close(fd[1]); //close write  
                kill(parentPid,SIGUSR1);
                childpids[kidnum] = 0;
                return 0;
            }   
        }

        else if (strncmp(firstArg,"list",4) == 0)
        {
            int childListed = 0;
            for (int i = 0; i < 10; i++)
            {
                if (childpids[i] != 0)
                {
                    childListed = 1;
                    printf("\nChild process %d - %s\n", i, childstatus[i]);
                }
            }
            if (childListed != 1)
            {
                printf("\nNo active children.\n");
            }
            printf("\n");

        }

        else if (strncmp(firstArg,"kill",4) == 0)
        {
            char secondArr[256];
            int secondArrExists = get_argument(input, 1, secondArr);
            if (secondArrExists == 0)
            {
                printf("\nError - please enter the number of the child process to be terminated.\n\n");
            }
            else if (atoi(secondArr) > 10 || atoi(secondArr) < 0)
            {
                printf("\nError - please restrict your number to a child process.\n\n");
            }
            else if (childpids[atoi(secondArr)] == 0)
            {
                printf("\nError - Child %d was already inactive.\n\n", atoi(secondArr));
            }
            else
            {

                kill(childpids[atoi(secondArr)], SIGKILL);
                printf("\nTerminated Child %d.\n\n", atoi(secondArr));
                childpids[atoi(secondArr)] = 0;
            }

        }

        else if (strncmp(firstArg,"quit",4) == 0 || (firstArg[0] == 'q' && firstArg[1] == '\n'))
        {
            int killPid;
            printf("\n-----------------------------------\n");
            printf("Exiting child processes...\n");
            for (int i = 0; i < 10; i++)
            {
                killPid = childpids[i];
                if (killPid != 0)
                {
                    kill(killPid, SIGKILL);
                    waitpid(killPid, 0, 0);
                }
            }
            printf("Exiting program...\n");
            return 0;
        }

        else if (strncmp(firstArg,"help",4) == 0)
        {
            printHelp();
        }

        else if (firstArg[0] == '\n')
        {
            fflush(0);
        }
        else if (overridemode == 0)
        {
            printf("\nError - Command not found. Type help to see all commands.\n\n");
        }

        //killing the kid for good:
        for(int i=0;i<10;i++) 
            if(childpids[i]!=0)       
                waitpid(childpids[i],0,WNOHANG);

                
    }
    return 0;
}