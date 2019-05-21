#ifndef __mapreduce_h__
#define __mapreduce_h__
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
typedef char *(*Getter)(char *key, int partition_number);
typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, Getter get_func, int partition_number);
typedef unsigned long (*Partitioner)(char *key, int num_buckets);

struct hashtable** partition_arr; //Partitions array
struct bucket** reducer_arr;      //Reducer array
Partitioner partitioner;
Mapper mapper;
Reducer reducer;
int partition_number;
int file_number;            // number of files and partitions
int current_file;
int current_partition;
//Bucket Element
struct bucket{
    char* key;
    struct bucket *next;
    struct linkedlist_node *linkedlist_node;
    struct linkedlist_node *current;
    int size;
};
//Hashtable
struct hashtable{
    int size;
    struct bucket **list;
    pthread_mutex_t *locks;
    long long bucketsize;
    pthread_mutex_t keylock;
};
//Value Linked List
struct linkedlist_node{
    char* val;
    struct linkedlist_node* next;
};

void MR_Emit(char *key, char *value);

unsigned long MR_DefaultHashPartition(char *key, int num_buckets);

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition);

#endif // __mapreduce_h__
