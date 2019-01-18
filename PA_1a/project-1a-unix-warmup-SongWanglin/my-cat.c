#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	if (argc <  2){
		exit(0);
	}
	FILE *fp;
	for (int i = 1; i<argc; i++){
		if (access(*(argv+i), F_OK)!=-1 ){
		   fp = fopen(*(argv+i),"r");
		} else if(*(argv+i)!=NULL){
	  	   printf("my-cat: cannot open file\n");
		   return 1;
		}
		char* buf = malloc(512*sizeof(char));
		while (fgets(buf, 512*sizeof(char), fp)!=NULL ){
			printf("%s", buf);
			memset(buf, 0, 512*sizeof(char));
		}

		free(buf);
	}
	exit(0);
}

