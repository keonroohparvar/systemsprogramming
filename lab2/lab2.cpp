#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct listelement {
    listelement *next,*prev;
    char text[1000];
};

listelement *head = NULL;

int main() {
    int userInput = 0;
    int c;
    while (userInput != 4) {

        printf("Select:\n1 push string\n2 print list\n3 delete item\n4 end program\n");
        scanf("%d", &userInput);
        
        do {
        c = getchar();
        } while(c != '\n' && c != EOF);
       //while (getchar() != '\n');

        //Push String
        if (userInput == 1) {
            char inputText[1000];
            printf("Enter your string: ");
            scanf("%s", inputText);
            printf("\n");

            //First Element
            if (!head) {
                //printf("Entered First\n");
                listelement *p = (listelement *) malloc(sizeof(listelement));
                if (!p) { return -1; }
                head = p;
                p -> prev = NULL;
                p -> next = NULL;
                strcpy(p -> text, inputText);
            }

            //Second Element and Onwards
            else {
                //printf("Entered Second\n");
                listelement *newlle = (listelement *) malloc(sizeof(listelement));
                listelement *p;
                for (p = head; p -> next != NULL; p = p -> next);
                p -> next = newlle;
                newlle -> prev = p;
                newlle -> next = NULL;
                strcpy(newlle -> text, inputText);
            }
        }

        // Print List
        else if (userInput == 2) {
            listelement *p;
            if (head == NULL) {
                printf("You have nothing in the list.\n");
            }
            else {
                for (p = head; p != NULL; p = p -> next)
                    printf("%s\n", p -> text);
            }
            printf("\n");
        }

        // Delete Item
        else if (userInput == 3) {
            int quitLoop = 0;
            int index;
            if (!head) {
                printf("There are no active nodes. Please add some before you delete!\n");
            }
            else {
                printf("Enter the index of the node to be removed: ");
                scanf("%d", &index);
                printf("\n");
                while (quitLoop != 1) {
                    listelement *p = head;
                    for (int i = 1; i < index; i++) {
                        if (!p) {
                            break;
                        }
                        else {
                            p = p -> next;
                        }
                        
                    }
                    if (p == NULL) {
                        printf("Cannot delete At the specified index.\n");
                        quitLoop = 1;
                    }
                    else if (p -> prev == NULL) {
                        head = head -> next;
                        head ? (head-> prev = NULL) : head = NULL;
                        quitLoop = 1;
                    }

                    else if (p -> next == NULL) {
                        p -> prev -> next = NULL;
                        quitLoop = 1;
                    }

                    else {
                        p -> prev -> next = p -> next;
                        p -> next -> prev = p -> prev;
                        quitLoop = 1;
                    }
                    printf("\n");
                }
            }
        }

        // End Program
        else if (userInput == 4) {
            return 1;
        }

        else {
            printf("Invalid input. Try again.\n");
        }
    }
    
    
    return 0;
}
