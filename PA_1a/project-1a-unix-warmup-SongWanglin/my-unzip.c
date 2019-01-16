#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    int number;
    char chr;

    if (argc == 1){ //no command-line arguments
        printf("my-unzip: file1 [file2 ...]\n");
        return 1;
    }
    for (int i = 1; i<argc; i++){ //go through all command line parameters except the first one
        FILE *file;
        file = fopen(argv[i], "rb");
        if (!file){
            printf("my-unzip: cannot open file\n");
            return 1;
        }
        while(fread(&number, sizeof(int), 1, file)){ //put 4 bytes from file inside number variable
            fread(&chr, sizeof(char), 1, file); //put 1 byte from file inside chr variable
            for (int i=0; i<number; i++) //print chr number times
                putc(chr, stdout);
        }
        fclose(file);
    }
    putc('\n', stdout);
}
