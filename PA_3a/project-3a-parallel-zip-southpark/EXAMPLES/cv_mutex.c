#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int *buffer;
int bufferSize;
int taken = 0;
int producerMaxSleep;
int consumerMaxSleep;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexP = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutexC = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condC = PTHREAD_COND_INITIALIZER;

void 	*Produce(void *arg);
void 	*Consume(void *arg);
void 	Display(int n);

int main(int argc, char** argv){
	if(argc < 4){
		printf("usage: [buffer size], [produce max sleep time[ms]], [consumer max sleep time[ms]]\n");
		exit(1);
	}
	bufferSize = atoi(argv[1]);
	producerMaxSleep = atoi(argv[2]);
	consumerMaxSleep = atoi(argv[3]);
	buffer = malloc(bufferSize*sizeof(int));
	for(int i = 0 ; i<bufferSize; i++){
		buffer[i] = 0;
	}
	srand(time(NULL));

	pthread_t producer, consumer;
	// should've check failure here
	pthread_create(&producer, NULL, Produce, NULL);
	pthread_create(&consumer, NULL, Consume, NULL);
	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);
	free(buffer);
	return 0;
}

void *Produce(void *arg){
	int counter = 1000;
	int indexProducer = 0;
	int sleepTime;
	while(1){
		sleepTime = rand()%producerMaxSleep;
		pthread_mutex_lock(&mutexP);
		if(taken == bufferSize){
			pthread_cond_wait(&condP, &mutexP);	//sem_wait
		}
		pthread_mutex_unlock(&mutexP);

		buffer[indexProducer] = counter++;		//Put

		pthread_mutex_lock(&mutex);
			taken++;				//sem_post(full_buffer)
		pthread_mutex_unlock(&mutex);
		
		indexProducer = (indexProducer + 1)%bufferSize;

		pthread_mutex_lock(&mutex);
			printf("%6d ->  ", counter-1);
			Display(bufferSize);
			printf("\n\n");
		pthread_mutex_unlock(&mutex);

		pthread_cond_signal(&condC);
		sleep(sleepTime/1000);
	}
}

void *Consume(void *arg){
	int indexConsumer = 0;
	int sleepTime;
	int tmp;
	while(1){
		sleepTime = rand()%consumerMaxSleep;
		pthread_mutex_lock(&mutexC);
		if(taken==0){
			pthread_cond_wait(&condC, &mutexC);	//sem_wait
		}
		pthread_mutex_unlock(&mutexC);

		tmp = buffer[indexConsumer];
		buffer[indexConsumer] = 0;			//GET

		pthread_mutex_lock(&mutex);
			taken--;				//sem_post(empty_buffer)
		pthread_mutex_unlock(&mutex);

		indexConsumer = (indexConsumer + 1)%bufferSize;

		pthread_mutex_lock(&mutex);
			printf("	    ");
			Display(bufferSize);
			printf("->  %6d  \n\n", tmp);
		pthread_mutex_unlock(&mutex);
		
		pthread_cond_signal(&condP);
		sleep(sleepTime/1000);
	}
}

void Display(int n){
	for(int i = 0; i<n; i++){
		printf("| %6d", buffer[i]);
	}
	printf("|");
}
