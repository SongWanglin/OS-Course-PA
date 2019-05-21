#include "mapreduce.h"
#define _GNU_SOURCE
#include <pthread.h>
int PARTITION_SIZE = 1543;              // size of one single partition table
pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *partition_lock;        //locks for file and partition

/* Hash table functions */
//Function used to instantiate the table
struct hashtable *create_table(int size){
    struct hashtable *ht = (struct hashtable*)malloc(sizeof(struct hashtable));
    ht->size = size;
    ht->list = malloc(sizeof(struct bucket*)*size);
    ht->locks=malloc(sizeof(pthread_mutex_t)*size);
    pthread_mutex_init(&ht->keylock,NULL);
    ht->bucketsize=0;
    for(int i=0;i<size;i++){
        ht->list[i] = NULL;
        if(pthread_mutex_init(&(ht->locks[i]), NULL)!=0){
            printf("Locks init failed!\n\n");
            assert(pthread_mutex_init(&(ht->locks[i]), NULL)==0);
        }
    }
    return ht;
}
/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
 unsigned long long hash(char *str)
 {
     unsigned long long hash = 5381;
     int c;
     while((c = *str++)!= '\0'){
         hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
     }
     hash=hash%PARTITION_SIZE;
     return hash;
 }
//insert value into the hash table.
void ht_put(struct hashtable *ht,char* key,char* val){
    long long bucket_index = hash(key);
    pthread_mutex_t *lock = ht->locks + bucket_index;       // locked the select bucket
    struct linkedlist_node *newlinkedlist_node=malloc(sizeof(struct linkedlist_node));
    newlinkedlist_node->val=strdup(val);
    newlinkedlist_node->next=NULL;
    pthread_mutex_lock(lock);
    struct bucket *list = ht->list[bucket_index];
    struct bucket *temp = list;
    // find the right linklist node in the bucket, do the put data and unlock the bucket
    while(temp!=NULL){
        if(strcmp(temp->key,key)!=0){
            temp = temp->next;
            continue;
        }
        struct linkedlist_node *sublist = temp->linkedlist_node;
        newlinkedlist_node->next=sublist;
        temp->size++;
        temp->linkedlist_node = newlinkedlist_node;
        temp->current = newlinkedlist_node;
        pthread_mutex_unlock(lock);
        return;
    }
    // if no existing key matched, add a new bucket in the partition for the input data
    pthread_mutex_lock(&ht->keylock);
    ht->bucketsize++;
    pthread_mutex_unlock(&ht->keylock);
    struct bucket *newbucket = malloc(sizeof(struct bucket));
    newbucket->key=strdup(key);
    newbucket->linkedlist_node=newlinkedlist_node;
    newbucket->current=newbucket->linkedlist_node;
    newbucket->next = list;
    ht->list[bucket_index] = newbucket;
    pthread_mutex_unlock(lock);
}
 //Method used for qsort to sort the list of keys in the partition.
 int keycmp(const void *s1, const void *s2){
     struct bucket **bucket1 = (struct bucket **)s1;
     struct bucket **bucket2 = (struct bucket **)s2;
     int nullable_key1 = (bucket1==NULL)? 0: 1;
     int nullable_key2 = (bucket2==NULL)? 0: 1;
     return (*bucket1==NULL || *bucket2==NULL)? nullable_key1-nullable_key2: \
        strcmp((*bucket1)->key,(*bucket2)->key);
 }
//Getter function that returns the next waiting key in a linkedlist_node list.
char* get_next(char* key, int partition_num){
    struct bucket *temp_bucket= reducer_arr[partition_num];
    struct linkedlist_node *cur_bucket = temp_bucket->current;
    if(cur_bucket==NULL){
        return NULL;
        }
    temp_bucket->current=cur_bucket->next;
    return cur_bucket->val;
}
// free table
void free_table(struct hashtable *ht){
    for(int i=0;i<ht->size;i++){
        struct bucket *list1=ht->list[i];
        struct bucket *temp2=list1;
        pthread_mutex_t *lock=&(ht->locks[i]);
        while(temp2!=NULL){
            struct bucket *temp_bucket=temp2;
            struct linkedlist_node *temp_node=temp_bucket->linkedlist_node;
            while(temp_node!=NULL){
                struct linkedlist_node *sublist=temp_node;
                temp_node=temp_node->next;
                free(sublist->val);
                free(sublist);
            }
            temp2=temp2->next;
            free(temp_bucket->key);
            free(temp_bucket);
        }
        pthread_mutex_destroy(lock);
    }
    free(ht->locks);
    free(ht->list);
    free(ht);
}

/* Mapper and reducer threads */
void *mapper_thread(void *files){               // find the file and call mapper to map it
    char **arguments = (char**) files;
    for(;;){
        // find the right file
        pthread_mutex_lock(&file_lock);
        int index = 0;
        if(current_file>file_number){
            pthread_mutex_unlock(&file_lock);
            return NULL;
        }
        if(current_file<=file_number){
            index=current_file;
            current_file++;
        }
        pthread_mutex_unlock(&file_lock);
        mapper(arguments[index]);
    }
    return NULL;
}
void* reduce_thread(){                         // find the right partition and do the reduce
    for(;;){
        // find the right partition from partition array
        pthread_mutex_lock(&file_lock);
        int partition_index = 0;
        if(current_partition>=partition_number){
            pthread_mutex_unlock(&file_lock);
            return NULL;
        }
        if(current_partition<partition_number){
            partition_index=current_partition;
            current_partition++;
        }
        pthread_mutex_unlock(&file_lock);
        if(partition_arr[partition_index]==NULL)
            return NULL;
        // find the target bucket in the partition
        struct hashtable* cur_partt=partition_arr[partition_index];
        struct bucket *list[ partition_arr[partition_index]->bucketsize ];
        long long list_index = 0;
        for(int i=0;i<PARTITION_SIZE;i++){
            if(cur_partt->list[i] ==NULL){
                continue;
            }
            struct bucket* temp_bucket=cur_partt->list[i];
            while(temp_bucket!=NULL){
                list[ list_index ]=temp_bucket;
                list_index++;
                temp_bucket=temp_bucket->next;
            }
        }
        // sort the keys and reduce
        qsort(list,partition_arr[partition_index]->bucketsize,sizeof(struct bucket *),keycmp);
        for(int i=0;i<list_index;i++){
            reducer_arr[partition_index]=list[i];
            reducer(list[i]->key,get_next,partition_index);
        }
    }
}

/* Mapreduce part */
//Default Hash function for partitioner
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0'){
        hash = ((hash << 5) + hash) + c;
    }
    return hash % num_partitions;
}
//EmitIntermediate(w, "1");
void MR_Emit(char *key, char *value){
    long partition_index = (partitioner!=NULL)? partitioner(key,partition_number)\
                    :MR_DefaultHashPartition(key,partition_number);
    ht_put( partition_arr[partition_index],key,value);
}
// Master thread that starts and joins the mapper/reducer threads
void MR_Run(int argc, char *argv[],
    Mapper map, int num_mappers,
    Reducer reduce, int num_reducers,
            Partitioner partition){
    current_partition=0;
    current_file=1;
    partitioner=partition;
    mapper=map;
    reducer=reduce;
    partition_number=num_reducers; //Number of partitions
    file_number=argc-1;
    partition_arr = malloc(sizeof(struct hashtable *)*num_reducers);
    //Create Partitions
    for(int i=0;i<partition_number;i++){
        partition_arr[i] = create_table(PARTITION_SIZE);
    }
    //Start and Join Mapping Process
    pthread_t mappers[num_mappers];
    for(int i=0;i<num_mappers || i==file_number;i++){
         pthread_create(&mappers[i], NULL,mapper_thread,(void*) argv);
    }
    for(int i=0;i<num_mappers || i==file_number;i++)//Start joining all the threads
    {
            pthread_join(mappers[i], NULL);
    }
    //Start and join reducing process
    pthread_t reducer[num_reducers];
    reducer_arr=malloc(sizeof(struct bucket *) * num_reducers);
    for(int i=0;i<num_reducers;i++){
        pthread_create(&reducer[i],NULL,reduce_thread,NULL);
    }
    for(int i=0;i<num_reducers;i++){
        pthread_join(reducer[i],NULL);
    }
    // Free Memory
    for(int i=0;i<partition_number;i++){
        free_table(partition_arr[i]);
    }
    free(partition_lock);
    free(reducer_arr);
    free(partition_arr);
}

