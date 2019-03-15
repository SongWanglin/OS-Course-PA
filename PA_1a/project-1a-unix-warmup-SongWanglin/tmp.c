#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    char last = '\0', current = '\0'; // 2 counters to search and then make changes (2-pointer method)
    int count = 1;  //how many times a character appear
    FILE *fp;

    /* check if argv provided any file names */
    if (argc == 1){ 
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    for (int i=1; i<argc; i++){ //go through all *argv[i] except the first one (which is supposed to be ./my-zip)
        fp = fopen(*(argv+i), "r");

	/* check if this filename is valid */
        if (!fp){   
            printf("my-zip: cannot open file\n");
            exit(1);
        }
        if (last == '\0')
            last = getc(fp);  // get the character from file stream, return the corresponding ASCII or EOF
	
	/* 2-pointer tracking */
        while( (current = getc(fp))!= EOF) { // use "current" counter to search
            if(current==last){  
                count++;    // count the same character
            } else{         // write to the output stream if we found the next different character, reset the counter to 1
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&last, sizeof(char),  1, stdout);
                count = 1;
            }
            last = current;
        } 

	/* check if it is the last char in  the last file, if so, write what's still stored in the current tracker */
	/* (the last character of other files is already handled by the previous while loop) */
        if( i == argc-1){ 
            fwrite(&count, sizeof(int), 1, stdout);
            fwrite(&last, sizeof(char), 1, stdout);
            count = 1;
        }
        fclose(fp);
    }
    return 0;
}
