#include <stdlib.h>
#include <stdio.h>


int main (int argc, char ** argv) {
	if (argc<2){
		printf("my-zip: file1 [file2 ...]\n");
		exit(1);
	}
	FILE *fp;
	int count=0;
	int r;
	int last;
	


	for (int i=1;i<argc;i++){
		fp=fopen(argv[i],"r");
		if (fp==NULL){
			printf("can not open file");
			exit(1);
		}
		
		r=fgetc(fp);
		if(i==1){
			
			last=r;
		}
		while (r!=EOF){
			if (r!=last){
				fwrite(&count,sizeof(int),1,stdout);
				fputc(last,stdout);
				count=1;
				last=r;
			}else{
				count++;
			}
			r=fgetc(fp);
		}		
		
		fclose(fp);
	}
		fwrite(&count,sizeof(int),1,stdout);
    		fputc(last,stdout);
		return 0;
}
