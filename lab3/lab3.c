#include <stdio.h>
#include <math.h>

//Have a global array of one MB
unsigned char myheap[1048576];
//Have a pagesize
#define PAGESIZE 1024

typedef struct chunkhead
{
unsigned int size;
unsigned int info;
unsigned char *next,*prev;
}chunkhead;

void init() {
    chunkhead *firstChunk = (chunkhead *) &myheap[0];
    firstChunk->size = 1048576 - sizeof(chunkhead);
    firstChunk->info=0;
    firstChunk->next = NULL;
    firstChunk->prev = NULL; 
}

unsigned char *mymalloc(unsigned int size) {
    int totalBytes = size + sizeof(chunkhead);
    int numPages = (totalBytes / PAGESIZE);
    if (totalBytes % PAGESIZE > 0)
        numPages++;
    chunkhead *currChunkHead = (chunkhead *) myheap;
    while (currChunkHead->info != 0) {
        currChunkHead = (chunkhead *)currChunkHead->next;
    }

    //Checks if there is enough space for the allocation
    if (currChunkHead->size < (numPages * PAGESIZE))
        return NULL;

    //Updates Current Chunkhead and allocates space
    currChunkHead->info = 1;
    unsigned char *returnPointer = (unsigned char*)currChunkHead + sizeof(chunkhead);

    //Creates new chunkhead
    chunkhead *newChunkHead = (chunkhead *)(returnPointer + (PAGESIZE * numPages));
    currChunkHead->next = (unsigned char *)newChunkHead;
    newChunkHead->prev = (unsigned char *)currChunkHead;
    newChunkHead->size = currChunkHead->size - (sizeof(chunkhead) + (PAGESIZE * numPages));
    newChunkHead->next = NULL;
    newChunkHead->info = 0;


    //Returns the pointer to beginning of allocated memory
    return returnPointer;

    
}

void myfree(unsigned char *address){
    // You said in lecture NOT to combine anything, simply just free the chunk 
    // the specified address. Thus, this is what I did; the memory management I 
    // will complete in the project. I also am assuming the address passed in 
    // is the first address of the chunk of memory.
    unsigned char *chunkPointer = address - sizeof(chunkhead);
    ((chunkhead *)chunkPointer)->info=0;
}

void analyse() {
    int chunkNum = 0;
    chunkhead *currChunk = (chunkhead *) myheap;
    for (currChunk; currChunk != NULL; currChunk = (chunkhead *) currChunk->next ) {
        if (currChunk == NULL) {
            printf("Done with Analyse");
            break;
        }
        chunkNum++;
        printf("Chunk #%d (at %p):\n", chunkNum, currChunk);
        printf("Size: %d\n", currChunk->size);
        (currChunk->info == 1) ? printf("Occupied\n") : printf("Empty\n");
        printf("Next: %p\n", currChunk->next);
        printf("Prev: %p\n", currChunk->prev);
        printf("\n\n");
    }
}

void main() {
    init();
    /*
    unsigned char *a, *b, *c;
    a = mymalloc(1000);
    b = mymalloc(1000);
    c = mymalloc(1000); 
    myfree(b);
    myfree(a);   
    */
    unsigned char *a;
    int i;
    a = mymalloc(100);
    chunkhead *b = (chunkhead *)mymalloc(100);
    chunkhead *c = (chunkhead *)mymalloc(2000);
    b = b - 1;
    c = c - 1;
    chunkhead *chunkPtr = ((chunkhead *)a);
    chunkPtr = chunkPtr - 1;
    int size = chunkPtr->size;
    printf("%d\n%d\n%d\n", size, b->size, c->size);
  
}