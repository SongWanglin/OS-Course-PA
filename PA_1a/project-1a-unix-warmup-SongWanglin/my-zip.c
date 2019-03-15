#include "defs.h"

int main(int argc, char* argv[]){
	check_err( (argc<2), "my-zip: file1 [file2 ...]\n");   
	chunk_size = (1024*1024)*2;			// chunk_size * q_size should be greater than input file size
	N_files=argc-1; 				// Number of files
	chunk_of_files=malloc(sizeof(int)*N_files); 	// an array to record the chunks per file.
	
	pthread_t producer;
	pthread_create(&producer, NULL, produce, argv+1);
    	pthread_join(producer,NULL);

	//mmap完毕，关门放CPU
	result=malloc(sizeof(struct compressed_chunk)* N_chunks); 
	 // Number of consumer threads 
        int N_threads= (get_nprocs()<=N_chunks)? get_nprocs(): N_chunks;
        pthread_t consumer[N_threads];	
    	for (int i = 0; i < N_threads; i++) {
        	pthread_create(&consumer[i], NULL, consume, NULL);
    	}

    	for (int i = 0; i < N_threads; i++) {
        	pthread_join(consumer[i], NULL);
    	}
	print_queue();
	return 0;
}   

void put(struct chunk input){
  	queue[nextIn] = input; 		//Enqueue the chunk
  	nextIn = (nextIn + 1) % q_size;
  	taken++;
}

struct chunk get(){
  	struct chunk res = queue[nextOut]; //Dequeue the chunk.
	nextOut = (nextOut + 1) % q_size;
  	taken--;
  	return res;
}

void* produce(void *argv){
	char** files = (char **)argv;
	struct stat f_stat;				// file status from fstat
	char* chs; 					// address: from mmap, contents: characters from input
	int f_desc, chunk_count,last_chunk_size;	// file descriptor from open & chunk count in one file & size of this file's last chunk
	
	for(int i=0;i<N_files;i++){
		f_desc = open(files[i], O_RDONLY);
		// number of chunks in this file + size of this file's last chunk
		chunk_count=0,last_chunk_size=0;
		check_err( (f_desc == -1),"my-zip: cannot open file\n");
        
        	fstat(f_desc, &f_stat);
		chunk_count = my_ceil((double)f_stat.st_size/chunk_size);
		last_chunk_size=(chunk_count > f_stat.st_size/chunk_size)? f_stat.st_size%chunk_size: chunk_size; // is the file size multiple of chunk size?
		chunk_of_files[i]=chunk_count;
		N_chunks+=chunk_count;
		
        	chs = mmap(NULL, f_stat.st_size, PROT_READ, MAP_SHARED, f_desc, 0); //If addr is NULL, then the kernel chooses the (page-aligned) address
		check_err( (chs == MAP_FAILED),"Error mmapping the file\n");
    	
		for(int j=0;j<chunk_count;j++){
			struct chunk product;
			product.character=chs;
			product.chunk_id=j;
			product.file_id=i;
			product.Size = (j==chunk_count-1)? last_chunk_size: chunk_size;
			chs+= chunk_size; 	// this chunk's done mapping, go to next chunk
			put(product);
		}
		close(f_desc);
	}
	return 0;
}

//basically p1a my-zip for every chunk
struct compressed_chunk rle_encoding(struct chunk input){
	struct compressed_chunk res;
	res.counts=malloc(input.Size*sizeof(int));	
	char* data=malloc(input.Size);
	int r_id=0, i = 0; 
	while( i<input.Size){
		data[r_id]= input.character[i];
		res.counts[r_id]=1;
		while(i+1<input.Size && input.character[i]==input.character[i+1]){
			res.counts[r_id]++;
			i++;
		}
		r_id++;i++;
	}
	res.size=r_id;
	res.chars = malloc(r_id*sizeof(char));
	memcpy(res.chars, data, sizeof(char)*r_id);
	return res;
}

void *consume(){
	do{
		int c_id = 0;
		pthread_mutex_lock(&q_lock);
		struct chunk input=get();
		pthread_mutex_unlock(&q_lock);
		// get the corresponding chunk id
		for(int i=0;i<input.file_id;i++){
			c_id+=chunk_of_files[i];
		}
		c_id+=input.chunk_id;
		result[c_id]= rle_encoding(input);
	}while(taken!=0);
	return NULL;
}

void print_queue(){
	char* start_addr=malloc(N_chunks*chunk_size*(sizeof(int)+sizeof(char)));
    char* end_addr= start_addr; // the ending address for printing
	for(int i=0;i<N_chunks;i++){
		int last_char_id = result[i].size-1;
		if(i<N_chunks-1 && result[i].chars[last_char_id]==result[i+1].chars[0]){ //Compare i'th output's last character with the i+1th output's first character
				result[i+1].counts[0]+=result[i].counts[last_char_id];
				result[i].size--;
		}
		
		for(int j=0;j<result[i].size;j++){
			*((int*)end_addr)=result[i].counts[j];
			end_addr+=sizeof(int);
            		*end_addr = result[i].chars[j];
            		end_addr+=sizeof(char);
		}
	}
	fwrite(start_addr,end_addr-start_addr,1,stdout);
}
