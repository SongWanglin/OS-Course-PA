///////// Passing all test cases /////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

#define Pthread_create(thread, attr, start_routine, arg)       assert(pthread_create(thread, attr, start_routine, arg) == 0);
#define Pthread_join(thread, value_ptr)        assert(pthread_join(thread, value_ptr) == 0);
#define Pthread_mutex_lock(m)                  assert(pthread_mutex_lock(m) == 0);
#define Pthread_mutex_unlock(m)                assert(pthread_mutex_unlock(m) == 0);
#define Pthread_cond_wait(c,m)                 assert(pthread_cond_wait(c,m) == 0);
#define Pthread_cond_signal(c)                 assert(pthread_cond_signal(c) == 0);

#define CHUNKSIZE 50000000
#define STORE_BUFFER_SIZE 1000000

typedef struct __attribute__((packed)) {
  int count;
  char c;
}RLE;

typedef struct{
  char *startRdAdr;
  char *startWrAdr;
  int *rleSlots;
}myarg_t;

myarg_t **buffer; // pointer to global buffer holding start addresses of the chunks to be worked on by consumers
int numfull = 0;
int fillptr = 0;
int useptr = 0;
int stopWork = 0;
int noOfProc = 1; // default set to 1
char *lastAdr = NULL;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void do_fill(myarg_t *Adr){
  buffer[fillptr] = Adr;
  fillptr = (fillptr + 1) % noOfProc;
  numfull++;
}

myarg_t* do_get(){
  myarg_t *Adr = buffer[useptr];
  useptr = (useptr + 1) % noOfProc;
  numfull--;
  return Adr;
}

void *doParaWork(void *arg){
  while(1){
    char matchChar = 0;
    int firstTime = 1;
    int i = 0;
    int count = 0;
    
    Pthread_mutex_lock(&m);
    while(numfull == 0)
      Pthread_cond_wait(&fill, &m);
    myarg_t *Adr = do_get();    
    Pthread_cond_signal(&empty);
    Pthread_mutex_unlock(&m);

    if(Adr == NULL){
      break;
    }
    
    // Start zip
    char *startAdr = Adr->startRdAdr;
    RLE *rle = (RLE *)Adr->startWrAdr;
    int *rleSlots2 = Adr->rleSlots;
    *rleSlots2 = 1;
    while(( (startAdr+i) != lastAdr) && (i < CHUNKSIZE) ){
      if(firstTime){
	firstTime = 0;
	matchChar = startAdr[i];
      }
    
      if(startAdr[i] == matchChar){
	count++;
      }
      else{
	rle->count = count;
	rle->c = matchChar;
	(*rleSlots2)++;
	count = 1;             
	matchChar = startAdr[i];
	rle++;
      }
      i++;
    }
    rle->count = count;
    rle->c = matchChar;
  }
  return 0;
}


int main(int argc, char *argv[])
{
  noOfProc = get_nprocs();
  
  if(argc == 1){
    printf("my-zip: file1 [file2 ...]\n");
    exit(1);   
  }
  else{
    RLE prevRle={0,0};
    int firstFlag = 1;
    RLE storeRle[STORE_BUFFER_SIZE];
    int indexS = 0;
    
    for(int indexFiles = 1; indexFiles < argc ; indexFiles++){
      FILE *fp = fopen(argv[indexFiles], "r");
      int fd = fileno(fp);      
      fseek(fp, 0L, SEEK_END);
      int fileSize = ftell(fp);
      char *fileMap =(char *)mmap(NULL, (size_t)fileSize, PROT_READ, MAP_SHARED, fd, 0L);
      char *fileMapStart = fileMap;
      fclose(fp);
      lastAdr = fileMap + fileSize;
      
      // Create consumer threads
      pthread_t *cid = (pthread_t *)malloc(noOfProc * sizeof(pthread_t));
      for(int i = 0; i <  noOfProc; i++){
	Pthread_create(&cid[i], NULL, doParaWork, NULL);
      }

      // Create a queue to hold start read and write addresses
      buffer = (myarg_t **)malloc(noOfProc * sizeof(myarg_t*));
            
      // Allocate memory for writing zipped output, buffer size will not exceed 5x fileSize
      char *writeBuffStartAdr = (char*)malloc(fileSize*6);

      // Filling the queue with read and write addresses
      char *writeBuff = writeBuffStartAdr;      
      int totalChunks = (fileSize + CHUNKSIZE - 1) / CHUNKSIZE;

      // Allocate memory for the count of used slots in a zipped output
      int *rleSlotsStartAdr = (int*)malloc(totalChunks*sizeof(int));
      int *rleSlots1 = rleSlotsStartAdr;

      for(int index = 0; index < totalChunks; index++ ){
	Pthread_mutex_lock(&m);
	while(numfull == noOfProc){
	  Pthread_cond_wait(&empty,&m);	  
	}
	
	myarg_t *myarg = (myarg_t *)malloc(sizeof(myarg_t*));
	myarg->startRdAdr = fileMap;
	myarg->startWrAdr = writeBuff;
	myarg->rleSlots = rleSlots1;
	do_fill(myarg);                   // Fill the read and write addresses in the queue
	fileMap += CHUNKSIZE;             // Shift the read address by the CHUNKSIZE
	writeBuff += CHUNKSIZE*6;         // Shift the write address by CHUNKSIZE*6 bytes
	rleSlots1++;
	Pthread_cond_signal(&fill);
	Pthread_mutex_unlock(&m);	
      }

      // Inserting NULL in the queue
      for(int index = 0; index < noOfProc; index++ ){	
	Pthread_mutex_lock(&m);
	while(numfull == noOfProc){
	  Pthread_cond_wait(&empty,&m);
	}
	do_fill(NULL);
	Pthread_cond_signal(&fill);
	Pthread_mutex_unlock(&m);	
      }

      // Waiting for the threads to join
      for(int i = 0; i <  noOfProc; i++){
	Pthread_join(cid[i], NULL);
      }
      
      int *rleSlotPtr = rleSlotsStartAdr;

      RLE currRle;
      for(int index = 0; index < totalChunks; index++){
	RLE *rlePtr = (RLE*)writeBuffStartAdr;

	for(int i = 0; i < *(rleSlotPtr); i++){
	  currRle = *rlePtr;
	  if(firstFlag){	    
	    firstFlag = 0;
	    prevRle = currRle;
	  }
	  else{
	    if(prevRle.c == currRle.c){
	      prevRle.count += currRle.count;	      
	    }else{	      
	      if(indexS == STORE_BUFFER_SIZE){
		fwrite(&storeRle[0],sizeof(storeRle),1,stdout);
		indexS = 0;
	      }		
	      storeRle[indexS] = prevRle;
	      indexS++;

	      prevRle = currRle;	      
	    }
	  }
	  rlePtr++;
	}      	
	writeBuffStartAdr = writeBuffStartAdr + (CHUNKSIZE*6);
	rleSlotPtr++;
      }
      munmap(fileMapStart, fileSize);
    } 
    storeRle[indexS] = prevRle;
    indexS++;
    fwrite(&storeRle[0],indexS*sizeof(storeRle[0]),1,stdout);
  }
  exit(0);
}
