#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

void synch(int par_id, int par_count, int *ready)
{
    int synchid = ready[par_count]+1; 
    ready[par_id]=synchid;
    int breakout = 0;
    while(1)
    {
        breakout=1;
        for(int i=0;i<par_count;i++)
        { 
            if(ready[i]<synchid)
            {
                breakout = 0;
                break;    
            } 
        }
        if(breakout==1) 
        {
            ready[par_count] = synchid; 
            break;
        } 
    }
}

void randomizeMatrix(float *matrix)
{
    srand(time(NULL));
    for (int i = 0; i < 32 * 32; i++)
    {
        /* Only including numbers 0 through 9 as was said in lecture */
        float randNum = (float)rand() / (float)RAND_MAX; 
        randNum = randNum * (float)10;
        matrix[i] = randNum;
    }   
}

void matrixMultiply(float *result, float *matrixA, float *matrixB, int progNum, int numProcesses)
{
    int division = 32 / numProcesses;
    int startingIndex = progNum * division;
    int sum, tempNum;
    for (int y=0; y<32; y++)
    {
        for (int x = startingIndex; x < (startingIndex + division); x++)
        {
            sum = 0;
            for (int z=0; z < 32; z++)
            {
                tempNum = matrixA[y*32 + z] * matrixB[x + z * 32];
                sum = sum + tempNum;
            }
            result[x + y*32] = sum;
        }
    }
}

void printMatrix(float *matrix)
{
    printf("_______________________________________________________\n");
    for (int y=0; y<32; y++)
    {
        for (int x=0; x<32; x++)
        {
            printf("%.1f ", matrix[x + y * 32]);
        }
        printf("\n");
    }
    printf("_______________________________________________________\n");
}


int main(int argc, char *argv[20])
{
    /* i is the specific process 
     * n is the number of total processes 
     */
    int i, n;
    i = atoi(argv[1]);
    n = atoi(argv[2]);

    /* Print Args */
    // for (int i = 0; i<argc; i++)
    // {
    //     printf("Argument %d: %s\n", i, argv[i]);
    // }

    /* Initialize Ready */
    int fd = shm_open("readyList", O_RDWR, 0777);
    ftruncate(fd, 16*sizeof(int));
    int *readyList = mmap(NULL, 16*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (i == 0)
    {
        for (int i=0; i<16; i++)
        {
            readyList[i] = 0;
        }
    }

    /* Create Matricies */
    int fdForA, fdForB, fdForC;
    int matrixSize = 32 * 32;
    if (i == 0)
    {
        
        fdForA = shm_open("AMatrix", O_RDWR|O_CREAT, 0777);
        ftruncate(fdForA, matrixSize*sizeof(float));
        fdForB = shm_open("BMatrix", O_RDWR|O_CREAT, 0777);
        ftruncate(fdForB, matrixSize*sizeof(float));
        fdForC = shm_open("CMatrix", O_RDWR|O_CREAT, 0777);
        ftruncate(fdForC, matrixSize*sizeof(float));
    }

    else
    {
        sleep(1);
        fdForA = shm_open("AMatrix", O_RDWR, 0777);
        fdForB = shm_open("BMatrix", O_RDWR, 0777);
        fdForC = shm_open("CMatrix", O_RDWR, 0777);
    }

    float *aMatrix = (float *)mmap(NULL, matrixSize*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fdForA, 0);
    float *bMatrix = (float *)mmap(NULL, matrixSize*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fdForB, 0);
    float *cMatrix = (float *)mmap(NULL, matrixSize*sizeof(float), PROT_READ|PROT_WRITE, MAP_SHARED, fdForC, 0);

    

    /* Initialize A and B Matrices with random numbers */
    if (i == 0)
    {
        randomizeMatrix(aMatrix);
        randomizeMatrix(bMatrix);
    }
    synch(i, n, readyList);

    /* Perform desired Matrix Multiplication */
    clock_t a;
    if (i == 0)
    {
        /* Start clock */
        a = clock();
    }
    matrixMultiply(cMatrix, aMatrix, bMatrix, i, n);
    synch(i, n, readyList);

    matrixMultiply(aMatrix, bMatrix, cMatrix, i, n);
    synch(i, n, readyList);

    matrixMultiply(bMatrix, aMatrix, cMatrix, i, n);
    synch(i, n, readyList);

    if (i == 0)
    {
        /* End clock */
        clock_t b = clock();
        double timeDiff = b - a;
        double timeInSec = timeDiff / CLOCKS_PER_SEC;
        
        /*Print Results - ALSO NOTE I AM ONLY PRINTING C AS WAS SAID IN PIAZZA */
        printf("Matrix C: \n");
        printMatrix(cMatrix);
        printf("\n");
        printf("Computation Time: %f\n", timeInSec);
    }


    /* Clean up */
    close(fdForA);
    close(fdForB);
    close(fdForC);
    munmap(aMatrix, matrixSize*sizeof(int));
    munmap(bMatrix, matrixSize*sizeof(int));
    munmap(cMatrix, matrixSize*sizeof(int));
    if (i == 0)
    {
        shm_unlink("AMatrix");
        shm_unlink("BMatrix");
        shm_unlink("CMatrix");
    }

    return 1;
}