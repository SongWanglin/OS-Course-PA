#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<regex.h>
#include<unistd.h>
#include<sys/types.h>

char *myname;
int ignore_case = 0;
int extended = 0;
int errors = 0;

regex_t pattern;

void process(const char *name, FILE *fp)
{
        char *buf = NULL;
        size_t size = 0;
        int ret;
        while (getline(& buf, &size, fp) != -1) {
                ret = regexec(& pattern, buf, 0, NULL, 0);
                if (ret != 0) {
                        if (ret != REG_NOMATCH){
                                free(buf);
                        errors ++;
                        return;
                        }
                }else{
                        printf("%s",  buf);
                }
        }
        free(buf);
}

int main(int argc, char **argv){
        int i;
        FILE *fp;
        myname = argv[0];
        /*while ((c = getopt(argc, argv, "(some chars)")) != -1) {
                switch (c) for more functionality
                }
        }*/
        if(optind == argc){
                printf("my-grep: searchterm [file ...]\n");
                exit(1);
        }
        /* compile pattern to see if it will success, return 1 if not*/
        int flags = REG_NOSUB, ret;
        if (ignore_case)
           flags |= REG_ICASE;
        if (extended)
           flags |= REG_EXTENDED;
        ret = regcomp (& pattern, argv[optind], flags);
        if (ret !=0 )
             errors++;
        if (errors)
                return 1;
        else
                optind++;
        /* search to find match */
        if(optind == argc)
                process("standard input", stdin);
        else{
                for (i = optind; i<argc; i++){
                        if(strcmp(argv[i], "-")==0)
                                process("standard input", stdin);
                        else if((fp=fopen(argv[i], "r"))!=NULL){
                                process(argv[i], fp);
                                fclose(fp);
                        } else{
                                printf("my-grep: cannot open file\n");
                                errors++;
                        }
                }
        }
        regfree (& pattern);
        return errors!=0;
}

