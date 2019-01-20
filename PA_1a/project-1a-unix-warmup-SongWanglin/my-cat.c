#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	/* check if enough arguments are provided */
	if (argc <  2){
		exit(0);
	}
	FILE *fp;
	/* open files */
	for (int i = 1; i<argc; i++){
		/* check if file name is valid */
		if (access(*(argv+i), F_OK)!=-1 ){
		   fp = fopen(*(argv+i),"r");
		}
	       	else if(*(argv+i)!=NULL){
	  	   printf("my-cat: cannot open file\n");
		   exit(1);
		}
		char* buf = malloc(512*sizeof(char));
		while (fgets(buf, 512*sizeof(char), fp)!=NULL ){
		    printf("%s", buf);
		}
		fclose(fp);
		free(buf);
	}
	return 0;
}

