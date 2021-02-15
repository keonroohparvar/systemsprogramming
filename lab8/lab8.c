#include <stdio.h>
#include <stdlib.h>



typedef unsigned char byte;

/* I am */
typedef struct mypipe
{
    byte* pipebuffer;
    int buffersize;
    int start_occupied;
    int end_occupied;
}mypipe;

/*initializes (malloc) the pipe with a size of "size" and sets start and end. */
void init_pipe(mypipe* pipe, int size)
{
    pipe->buffersize = size;
    pipe->pipebuffer = (byte *) malloc(size);
    pipe->start_occupied = pipe->end_occupied = 0;
}



/* writes "size" bytes from buffer into the pipe, returns size */
int mywrite(mypipe *pipe, byte* buffer, int size)
{
    int charIndex;
    char thisChar;
    for (charIndex = 0; charIndex < size; charIndex++)
    {
        thisChar = *(buffer + charIndex);
        if (thisChar == '\0')
        {
            pipe->pipebuffer[pipe->end_occupied] = thisChar;
            pipe->end_occupied = (pipe->end_occupied + 1) % pipe->buffersize;
            return charIndex;
        }
        (pipe->pipebuffer)[pipe->end_occupied] = *(buffer + charIndex);
        pipe->end_occupied = (pipe->end_occupied + 1) % pipe->buffersize;
        if ((pipe->end_occupied + 1) % pipe->buffersize == pipe->start_occupied)
        {
            return charIndex;
        }
    }
    pipe->end_occupied = (pipe->end_occupied + 1) % pipe->buffersize;
    pipe->pipebuffer[(pipe->start_occupied + size) % pipe->buffersize] = '\0';
    return size;
}



/* reads "size" bytes from pipe into buffer, returns how much it read (max size), 0 if pipe is empty */
int myread(mypipe* pipe, byte* buffer, int size)
{
    int charIndex;
    char thisChar;
    for (charIndex = 0; charIndex < size; charIndex++)
    {
        thisChar = pipe->pipebuffer[pipe->start_occupied];
        if (thisChar == '\0')
        {
            buffer[charIndex] = thisChar;
            pipe->start_occupied = (pipe->start_occupied + 1) % pipe->buffersize;
            char nextChar = pipe->pipebuffer[pipe->start_occupied];
            return charIndex;
        }
        buffer[charIndex] = thisChar;
        pipe->pipebuffer[pipe->start_occupied] = '\0';
        if (((pipe->start_occupied + 1) % pipe->buffersize) == pipe->end_occupied)
        {
            buffer[charIndex + 1] = '\0';
            pipe->start_occupied = pipe->end_occupied;
            return charIndex;
        }
        pipe->start_occupied = (pipe->start_occupied + 1) % pipe->buffersize;
    }
    pipe->start_occupied = (pipe->start_occupied + 1) % pipe->buffersize;
    buffer[size] = '\0';
    return size;
}

int main()
{
    char text[100];
    char currBuffer[35];
    mypipe pipeA;
    init_pipe(&pipeA, 32);
    mywrite(&pipeA, (byte *)"hello world", 12); 
    mywrite(&pipeA, (byte *)"it's a nice day", 16);
    myread(&pipeA, (byte *)text, 12); 
    printf("%s\n", text); 
    myread(&pipeA, (byte *)text, 16); 
    printf("%s\n", text);
    mywrite(&pipeA, (byte *)"and now we test the carryover", 30); 
    myread(&pipeA, (byte *)text, 30);
    printf("%s\n", text);
}