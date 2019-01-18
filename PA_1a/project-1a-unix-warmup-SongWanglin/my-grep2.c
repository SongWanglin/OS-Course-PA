#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main (int argc, char ** argv) {
    FILE *fp;;
    char *newline;
    int i, count = 0, occ = 0;
    if (argc <3){
	printf("my-grep: searchterm [file ...]\n");
	exit(1);
    }
    fp = fopen( *(argv+2), "r");
    if (fp == NULL){
	printf("my-grep: cannot open file\n");
        exit(1);
    }
    while (1){
	    char* fline = malloc(512*sizeof(char));
	    if (fgets(fline, 100, fp)==NULL){
		    break;
	    }
	    count++;
	    if (newline = strchr(fline, '\n'))
   	  	*newline = '\0';
	    if (strstr(fline, *(argv+1))!=NULL){
		printf("%s\n", fline);
		free(fline);
		occ++;
	    }
	}
    return 0;
}
