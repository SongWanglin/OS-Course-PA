#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *b_l(void *arg){
    char *p = (char*)arg;
    printf("%c\n", *p);
    return NULL;
}


int main(int argc, char** argv){
    pthread_t  p[5];
    char *c = malloc(sizeof(char));
    for(int  i = 0; i<5; i++){
        *c = 'a'+i;
        pthread_create(&p[i], NULL, b_l, (void *)c);
    }
    for(int i = 0; i<5; i++){
        pthread_join(p[i], NULL);
    }
    //printf("%d\n", *result);
    return 0;
}
