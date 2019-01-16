#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    int buffer;
    int temp;
    int counter;
    int lever;

    if (argc == 1){ //no command-line arguments
        printf("my-zip: file1 [file2 ...]\n");
        return 1;
    }
    for (int i=1; i<argc; i++){ //go through all command line parameters except the first one
        FILE *file;
        file = fopen(argv[i], "r");
        if (!file){
            printf("my-zip: cannot open file\n");
            return 1;
        }
        counter = 1;
        lever = 0;
        while ((buffer = fgetc(file)) != EOF){ //take one character from file
            if (!lever){ //only first cycle
                lever++;
                temp = buffer; //save character to temp
            }
            else if (lever){ //all other cycles
                if (buffer == temp) //check if character is same as previous
                    counter++;
                else{ //if different character - write out previous character's number and character itself
                    fwrite(&counter, sizeof(int), 1,  stdout);
                    fwrite(&temp, sizeof(char), 1, stdout);
                    counter = 1;
                    temp = buffer; //save different character to temp
                }
            }
        }
        fclose(file);
    }
}
