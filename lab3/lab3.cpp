#include <stdio.h>

//Have a global array of one MB
unsigned char myheap[1048576];
//Have a pagesize
#define PAGESIZE 1024

typedef struct chunkhead
{
unsigned int size;
unsigned int info;
unsigned char *next,*prev;
} chunkHeader;

chunkHeader *firstChunk = (chunkHeader *) &myheap[0];
firstChunk->size = 1048576 - sizeof(chunkHeader);
firstChunk->info=0;
firstChunk->next = NULL;
firstChunk->prev = NULL; 

unsigned char *mymalloc(unsigned int size) {
    int numPages = ((size + sizeof(chunkhead)) % PAGESIZE) + 1; //Including sizeof(chunkhead) for the next one
    chunkead *currChunkHead = myheap;
    while (currChunkHead->info != 0) {
        currChunkHead = currChunkHead->next;
    }

    //Checks if there is enough space for the allocation
    if (currChunkHead->size < (numPages * PAGESIZE))
        return NULL;


    //Updates Current Chunkhead and allocates space
    currChunkHead->info = 1;
    unsigned char *returnPointer = (unsigned char*)currChunkHead + sizeof(chunkhead);

    //Creates new chunkhead
    chunkhead *newChunkHead = returnPointer + (PAGESIZE * numPages);
    newChunkHead->prev = currChunkHead;
    newChunkHead->size = newChunkHead->prev->size - (sizeof(chunkhead) + (PAGESIZE * numPages));
    newChunkHead->next = NULL;
    newChunkHead->info = 0;


    //Returns the pointer to beginning of allocated memory
    return returnPointer;

    
}

void myfree(unsigned char *address){
}

void analyse() {
    int chunkNum = 0;
    chunkhead *currChunk = (chunkhead *) myheap;
    for (currChunk; currChunk != NULL; currChunk = currChunk->next ) {
        chunkNum++;
        printf("Chunk #%d:\n", chunkNum);
        printf("Size: %d", currChunk->size);
        (currChunk->info == 0) ? printf("Occupied\n") : printf("Empty\n");
        printf("Next: %p\n", currChunk->next);
        printf("Prev: %p\n", currChunk->prev);
        printf("\n\n");
    }
}

void main() {
    unsigned char *a,*b,*c; 
    a = mymalloc(1000);
    b = mymalloc(1000);
    c = mymalloc(1000);     
    analyse();
}