#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
	FILE *fp;
	char *temp = NULL;		 //one line  buffer from input stream 
	size_t length = 0; //status of read input, and  length of the line

	/* exception handling */
	if(argc == 1){
	    printf("my-grep: searchterm [file ...]\n");
	    exit(1);
	}

	/* grep stdin */
	if(argc ==2 ){
	   while(getline(&temp, &length, stdin)!=-1){
		if( !strstr(temp,  *(argv+1)) )
		{   continue; }
		printf("%s", temp);
	   } 
	}
	
	/* search in files */
	if(argc>2){
	    for (int i = 2; i<argc; i++){
		fp = fopen( *(argv+i), "r");
		/* check if file name valid */
		if (!fp){
		    printf("my-grep: cannot open file\n");
		    exit(1);
		    }
		/* compare file input line by line */
		while(getline(&temp, &length, fp)!=-1){
		    if( !strstr(temp, *(argv+1)) )
		    {	continue; }
		    printf("%s", temp);
		}
		fclose(fp);
	    }
	}
	/* remember to free temp (if allocated by getline function) */
	if(temp!=NULL) free(temp);	
	return 0;
}
