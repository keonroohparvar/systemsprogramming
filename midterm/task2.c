#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char firstName[20] = "Keon";
    char middleName[20] = "Sebastian";
    char lastName[20] = "Roohparvar";

    if (fork() == 0) {
        if (fork() == 0) {
            printf("%s ", firstName);
            return 1;
        }
        else {
            wait(0);
            printf("%s ", middleName);
        }
        return 1;
    }
    wait(0);
    printf("%s", lastName);
    return 1;
}