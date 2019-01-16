#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char ** argv) {
    /* if input string  arguments are less than 2, exit*/
    if (argc<2){
      exit(0);
    }
    FILE *fp;
    /* the *(argv) is the name of my-cat*/
    for (int i = 1; i<argc; i++){
      /* open all files if they do exist */
      if (access(*(argv+i), F_OK)!=-1){
        fp = fopen(*(argv+i), "r");
      } else if (*(argv+i)!=NULL){
        printf("my-cat: cannot open file\n");
      }
      /* allocate string buffer for printing file lines */
      char* buf = malloc(512*sizeof(char));
      while(fgets(buf, 512*sizeof(char), fp)!=NULL){
        printf("%s", buf);
        /* set the chunk of memory to all 0s before further use or free */
        memset(buf, 0, 512*sizeof(char));
      }
      free(buf);
    }
    return 0;
}
