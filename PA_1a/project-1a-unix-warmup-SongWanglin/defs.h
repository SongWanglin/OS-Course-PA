#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/mman.h> 
#include <sys/stat.h> 
#include <sys/sysinfo.h>
#include <unistd.h>

#include <pthread.h>

#define q_size (128) 	//Circular queue size. 
//required for the compressed output
int N_files, N_chunks, chunk_size; 
int* chunk_of_files;		//track chunk numbers of every page
//number of slots taken; circular queue head and tail.
int taken = 0, nextIn=0, nextOut=0; 
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;


struct chunk {
    char* character; 	// base_address of mmap + file characters
    int file_id; 	// file id
    int chunk_id; 	// chunk id
    int Size; 		// size of the Chunk
};

struct chunk queue[q_size];	// circular buffer

struct compressed_chunk {
	char* chars;
	int* counts;
	int size;
};				

struct compressed_chunk *result;	// final result: an array of compressed chunks


void put(struct chunk b);
struct chunk get();
void* produce(void *argv);
void *consume();
struct compressed_chunk rle_encoding(struct chunk input);
void print_queue();

void check_err(int exp, const char* msg){
	if(exp){
		printf("%s", msg);
		exit(1);
	}else{}
}

int my_ceil(double input){
	int res = (int)input;
	return (res<input)? (res+1): res;
}
