#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    int number = 0; // how many times the character repeat
    char chr = '\0'; //the repeated character
    FILE *fp;

    if (argc == 1){ // check if file names are provided
        printf("my-unzip: file1 [file2 ...]\n");
        return 1;
    }
    for (int i = 1; i<argc; i++){ 
        fp = fopen(*(argv+i), "r"); // open **argv one by one
        if (!fp){	// check if file names are valid
            printf("my-unzip: cannot open file\n");
            return 1;
        }
        while(fread(&number, sizeof(int), 1, fp)){ // put a 4B integer from file inside number variable, break if fail
            if( fread(&chr, sizeof(char), 1, fp)!=sizeof(char)) // break if can't read  a character from file
		    break;
	    for (int i=0; i<number; i++) // put number of chr to stdout
		    putchar(chr);
        }
        fclose(fp);
    }
    return 0;
}
