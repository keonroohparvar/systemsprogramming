#include <stdio.h>
#include <string.h>

int main()
{
    char testStr[256];
    testStr[0] = 'c';
    testStr[1] = 'a';
    testStr[2] = 't';
    testStr[3] = '\0';
    printf("Len: %ld\n", strlen(testStr));
    //test12345
}

