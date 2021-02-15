#include <stdio.h>

void main() {
    int leaves;
    int totalLeaves;
    int padding;
    int middleLeaf;
    char c = '*';
    do {
        printf("How many leaves would you like: ");
        scanf("%d", &leaves);
    } while (leaves > 10);

    totalLeaves = (2 * leaves) - 1;
    padding = (totalLeaves - 1)/2;
    middleLeaf = leaves - 1;  
    
    int i;
    int j;
    int k;
    for (i = 0; i < leaves; i++ ){
        for (j = 0; j < padding; j++) {
            printf(" ");
        }
        for (k = 0; k < 1 + 2 * i; k++)
            printf("%c", c);
        padding = padding - 1;
        printf("\n");
    }

    
    for (i = 0; i < 3; i++){
        for (j = 0; j < middleLeaf; j++) {
            printf(" ");
        }
        printf("%c\n", c);
    }
}
