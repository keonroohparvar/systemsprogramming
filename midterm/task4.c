#include <stdio.h>
#include <stdlib.h>     // srand, rand 
#include <time.h>       // time 
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>


int main ()
{
    srand (time(NULL)); // initialize random seed. That's important and comes right at the start of the main function and only once, never again.
    int *array = (int *)mmap(NULL, (100000 * sizeof(int)), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANON, -1, 0);
    int *parentCount = (int *)mmap(NULL, 100000, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANON, -1, 0);
    *(parentCount) = 0;
    int *childCount = (int *)mmap(NULL, 100000, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANON, -1, 0);
    *(childCount) = 0;
    //Note that i am storing wayyyy more space than I need to for the child/parent count
    //This is so that I will have a ton of room when I increment them
    
    
    /* Also parent doesn't need a mmap but I was lasy and I don't reme,ber the #include for 
    malloc, please don't hate me */
    
    //Initializing the array
    int i;
    int randomNumber;
    for (i = 0; i < 100000; i++) {
        randomNumber = rand() % 10;
        array[i] = randomNumber;
    }

    
    if (fork() == 0){
        // Child iterates through the bottom half
        for (i = 0; i < 50000; i++) {
            if (array[i] == 3)
                *(childCount) = *(childCount) + 1;
        }
        return 1;
    }
    else {
        // Parent iterates through the top half
        for (int i = 50000; i < 100000; i++) {
            if (array[i] == 3)
                *(parentCount) = *(parentCount) + 1;
        }
        wait(0); 
        // The wait is after the computation so that the two processes can 
        // run in parallel. 
    }

    int sum = *(parentCount) + *(childCount);
    printf("The number of 3's found in the array is: %d\n", sum); 

    munmap(array);
    munmap(parentCount);
    munmap(childCount);
    
    
    return 1;

}