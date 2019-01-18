#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    char last = '\0', current = '\0';
    int count = 1;  //how many times a character appear
    FILE *fp;
    if (argc == 1){ //no command-line arguments
        printf("my-zip: file1 [file2 ...]\n");
        return 1;
    }
    for (int i=1; i<argc; i++){ //go through all command line parameters except the first one
        fp = fopen(*(argv+i), "r");
        if (fp == NULL){
            printf("my-zip: cannot open file\n");
            return 1;
        }
        if (last == '\0')
            last = getc(fp);
        while( (current = getc(fp))!= EOF){
            if(current==last){
                count++;
            } else{
                fwrite(&count, 4, 1, stdout);
                fwrite(&last, 1,  1, stdout);
                count = 1;
            }
            last = current;
        }
        if( i == argc-1){
            fwrite(&count, 4, 1, stdout);
            fwrite(&last, 1, 1, stdout);
            count = 1;
        }
        fclose(fp);
    }
    return 0;
}
