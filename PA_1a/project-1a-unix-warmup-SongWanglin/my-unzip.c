#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    int number[1]; // how many times the character repeat
    char chr[1]; //the repeated character
    FILE *fp;

    if (argc == 1){ 
        printf("my-unzip: file1 [file2 ...]\n");
        return 1;
    }
    for (int i = 1; i<argc; i++){ 
        fp = fopen(argv[i], "r");
        if (!fp){
            printf("my-unzip: cannot open file\n");
            return 1;
        }
        while(fread(&number, sizeof(int), 1, fp)){ //put 4 bytes from file inside number variable
            if( fread(&chr, sizeof(char), 1, fp)!=1)
		    break;
	    for (int i=0; i<number[0]; i++)
		    putc(chr[0], stdout);
        }
        fclose(fp);
    }
    return 0;
}
