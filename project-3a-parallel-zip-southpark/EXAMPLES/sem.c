#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

sem_t empty_buffer;
sem_t full_buffer;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int *buf;
int LEN;
int nextOut = 0;
int nextIn = 0;
int item = 1;

void	put(int input);
int	get();
void	*producer(void *arg);
void	*consumer(void *arg);
void	print_buf();

int main(int argc, char** argv){
	if(argc < 4){
		printf("usage: [lenght of buffer],[number of producer], [number of consumer]\n");
		exit(1);
	}
	LEN = atoi(argv[1]);
	buf = malloc(LEN*sizeof(int));

	for(int i = 0; i<LEN; i++){
		buf[i] = 0;
	}

	int N_prod = atoi(argv[2]), N_cons = atoi(argv[3]);

	sem_init(&empty_buffer, 0, 0);
	sem_init(&full_buffer, 0, LEN-1);
	int rc = 0;
	
	printf("The initial circular queue:\n");	
	print_buf();
	printf("start the production/consumption:\n");

	pthread_t produce[N_prod], consume[N_cons];
	for(int i = 0; i<N_prod; i++){
		rc = pthread_create(&produce[i], NULL, &producer, NULL);
		assert(rc == 0);
	}
	for(int i = 0; i<N_cons; i++){
		rc = pthread_create(&consume[i], NULL, &consumer, NULL);
		assert(rc == 0);
	}
	
	for(int i = 0; i<N_prod; i++){
		pthread_join(produce[i], NULL);
	}

	for(int i = 0; i<N_cons; i++){
		pthread_join(consume[i], NULL);
	}

	free(buf);	
	return 0;
}

void put(int input){
	buf[nextIn] = input;
	nextIn = (nextIn + 1)%LEN;
}

int get(){
	int res = buf[nextOut];
	buf[nextOut] = 0;
	nextOut = (nextOut+1)%LEN;
	return res;	
}

void *producer(void *arg){
	while(1){
		sem_wait(&empty_buffer);
		pthread_mutex_lock(&mutex);
		put(item++);
		pthread_mutex_unlock(&mutex);
		
		printf("					%d  -> ", item);
		print_buf();
		sem_post(&full_buffer);
		sleep( 1*(rand()%3) );
	}
}

void *consumer(void *arg){
	int res = 0;
	while(1){
		sem_wait(&full_buffer);
		pthread_mutex_lock(&mutex);
		res = get();
		pthread_mutex_unlock(&mutex);
		if(res!=0){
			printf("%d  <- ", res);
			print_buf();
		}
		sem_post(&empty_buffer);
		sleep( 1*(rand()%3) );
	}
}

void print_buf(){
	printf("|");
	for(int i = 0; i<LEN; i++){
		printf(" %d |", buf[i]);
	}
	printf("\n");
}
