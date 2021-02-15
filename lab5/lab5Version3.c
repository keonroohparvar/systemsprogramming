#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main() {
    if (fork() == 0) {
        if (fork() == 0) {
            printf("1");
            return 1;
        }
        wait(0);
        printf("2");
        return 1;
    }
    wait(0);
    if (fork() == 0) {
        printf("3");
        return 1;
    }
    wait(0);
    printf("4");


    return 1;
}