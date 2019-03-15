/**********************************************************
 * Your Name:    Junrui Liu
 * Partner Name: Shihan Zhao
 *********************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdint.h> // portable int types
#include <fcntl.h> // open
#include <unistd.h> // close
#include <stdlib.h> // exit
#include <sys/stat.h> // file stat
#include <sys/mman.h> // mmap
#include <pthread.h> // pthreads
#include <semaphore.h> // semaphores
#include <sys/sysinfo.h> // get_nprocs (Linux)



/*********************************************************
 *                      Constants
 *********************************************************/


/* parameters. modify as you want */
#define MAX_CHUNCK_SIZE (1024*1024) // 1MB chuncks
// #define N_CPUS 6 // uncomment this to force # of cores
#define SLOTS_PER_CPU 10.0 // N_CPUS * 10MB buffer

/* macro functions */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* constants. DO NOT change them */
#define MAX_COUNT (1 << 31) // max count = 2^32
#define UNIT_SIZE 5 // 5 bytes per compressed unit

/* output behaviors */
#define WRITE_TO_STD
// #define WRITE_TO_FILE 0 // write to a file instead of stdout
#define OUT_FILE "./out"

/* for debugging */
#define print_flow printf


/*********************************************************
 *                   Type Declarations
 *********************************************************/

/* circular buffer */
typedef struct __buf_t {
    char** data;      // actual data buffer
    sem_t* compressed; // C tells W that it finishes compressing its slot
    sem_t* freed;      // W tells C that the requested slot is free
    int n_slots;       // # of slots
} buf_t;

/* args to C threads */
typedef struct __C_arg_t {
    int id;          // between 0 and n_slots
} C_arg_t;

/* no args to W thread */

/*********************************************************
 *                   Shared Meta-data
 *********************************************************/
char* file_start;
char* file_end;
buf_t* buf;
int compressors; // # of C threads
long chunck_size;
long bytes_per_slot;
long n_chuncks;

/*********************************************************
 *                 Function Declarations
 *********************************************************/

void* C (void *arg);
void* W (void *arg);
void work (int fd);
uint64_t get_file_size (int fd);
uint64_t get_total_ram ();
void write_to_buf (uint32_t count, char ch, char* ptr);
void write_out (uint32_t count, char ch, FILE* out);
void exit_if (int boolean, const char* msg);

/*********************************************************
 *                          Main
 *********************************************************/

double ceil(double input){
    int res = (int)input;
    if(res < input){
        return res + 1;}
    return res;
}

int main (int argc, const char* argv[])
{
    // take in a file name
    exit_if (argc != 2, "please specify exactly one input file");
    const char* filename = argv[1];
    
    // open the file
    int fd = open (filename, O_RDONLY);
    exit_if (fd < 0, "cannot open input file");

    // actual work
    work (fd);

    // close the file
    int rc = close (fd);
    exit_if (rc != 0, "cannot close input file");
}

void* C (void *arg)
{
    /* unpack the args */
    C_arg_t a = *(C_arg_t *) arg;
    int id = a.id;

    int chunck_i = id;
    char* input_ptr = file_start + id * chunck_size;
    while (input_ptr < file_end)
    {
        int slot = chunck_i % buf->n_slots; // current buffer slot
        
        /* wait until this buffer slot is free */
        sem_wait (&buf->freed[slot]);

        /* slot is free. begin compressing */
        char* slot_ptr = buf->data[slot]; // current pos in slot
        char* slot_end = slot_ptr + bytes_per_slot;
        char* chunck_end = MIN (input_ptr + chunck_size, file_end);

        /* compress a chunck of input data */
        // madvise (input_ptr, chunck_size, MADV_SEQUENTIAL);

        char prev_ch = -1;
        uint32_t count = 0;

        while (input_ptr < chunck_end)
        {
            char ch = *input_ptr; // read char
            input_ptr++;
            if (count == 0) // initialize
            {
                prev_ch = ch;
                count = 1;
            }
            else if (count < MAX_COUNT && ch == prev_ch) // combo
            {   count += 1;   }
            else // counter full, or a different char
            {   
                write_to_buf (count, prev_ch, slot_ptr);
                slot_ptr += UNIT_SIZE;
                prev_ch = ch;
                count = 1;
            }
        }
  //      if (count > 0) // last compressed unit hasn't been written to buf
    //    {
            write_to_buf (count, prev_ch, slot_ptr);
            slot_ptr += UNIT_SIZE;
//        }
        // if slot is not full, write a (0,0) pair signaling the EOF
        if (slot_ptr < slot_end)
        {   write_to_buf (0, 0, slot_ptr);   }

        // tell W that job is done
        sem_post (&buf->compressed[slot]);

        // go to the next chunck
        input_ptr += chunck_size * (compressors - 1);
        chunck_i += compressors; 
    }
    return 0;
}

void* W (void *arg)
{
    FILE* out;
    #ifdef WRITE_TO_FILE
        out = fopen (OUT_FILE, "w");
        exit_if (out == NULL, "cannot open output file");
    #else
        out = stdout;
    #endif

    /* transfer data from buffer to output file */
    int chunck_i = 0;
    char last_ch = 0; // buffer a unit, in case combos onto the next chunck
    uint32_t last_count = 0;

    while (chunck_i < n_chuncks)
    {
        int slot = chunck_i % buf->n_slots;

        /* wait for C to compress this chunck */
        sem_wait (&buf->compressed[slot]);

        /* good to go */
        char* slot_ptr = buf->data[slot]; // current pos in slot
        char* slot_end = slot_ptr + bytes_per_slot;

        while (slot_ptr < slot_end)
        {
            /* read the count and the ch */
            uint32_t count = *(uint32_t *) slot_ptr;
            slot_ptr += sizeof (uint32_t);
            char ch = *slot_ptr;
            slot_ptr++;
            if (last_count == 0) // initialize
            {
                last_count = count;
                last_ch = ch;
            }
            else if (count == 0) // EOF marked by C, can break early
            {   break;   }
            else if (ch != last_ch) // write out if no longer on combo
            {
                write_out (last_count, last_ch, out);
                last_count = count;
                last_ch = ch;
            }
            else // combo continues!
            {
                uint64_t temp_count = last_count + count;
                if (temp_count > MAX_COUNT) // overflow
                {
                    write_out (MAX_COUNT, last_ch, out);
                    last_count = temp_count - MAX_COUNT;
                }
                else // combo and no overflow!
                {   
                    last_count = last_count + count;
                }
            }
        }
        sem_post (&buf->freed[slot]);  // tell C that this slot is free
        chunck_i++;  // move on to next chunck (and slot)
    }
    
    if (last_count != 0)
    {   write_out (last_count, last_ch, out);  } // write out the very last unit
    
    #ifdef WRITE_TO_FILE
        int rc = fclose (out);
        exit_if (rc != 0, "cannot close output fd");
    #endif
    return 0;
}

void work (int fd)
{
    int rc = -1; // for return codes

    /* mmap the file */
    uint64_t file_size = get_file_size (fd);
    file_start = mmap (NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    file_end = file_start + file_size;
    exit_if (file_start == MAP_FAILED, "cannot mmap input file");

    /* partition the input file into chuncks */
    chunck_size = MIN (file_size, MAX_CHUNCK_SIZE);
    n_chuncks = (uint64_t) ceil (file_size * 1.0 / chunck_size);
    bytes_per_slot = chunck_size * UNIT_SIZE;

    /* create a circular buffer */
    buf = malloc(sizeof(buf_t));
    exit_if(buf == NULL, "cannot malloc buffer\n");

    int cpu_cores =
        #ifdef N_CPUS // if we have specified this parameter, use it
            N_CPUS;
        #else
            get_nprocs ();
        #endif
    exit_if (cpu_cores < 1, "needs at least one core");

    buf->n_slots = (int) ceil (SLOTS_PER_CPU * (cpu_cores - 1)); // 1 core reserved for W
    
    buf->data = malloc (buf->n_slots * sizeof (char*)); // ptrs to slot memory
    for (int i = 0; i < buf->n_slots; i++)
    {   
        buf->data[i] = malloc (bytes_per_slot); // slot memory
        exit_if (buf->data[i] == NULL, "cannot allocate buffer slots");
    }
    
    buf->compressed = malloc (buf->n_slots * sizeof (sem_t));
        exit_if (buf->compressed == NULL, "cannot allocate compressed semaphores array");
    buf->freed = malloc (buf->n_slots * sizeof (sem_t));
        exit_if (buf->freed == NULL, "cannot allocate freed semaphores array");

    for (int i = 0; i < buf->n_slots; i++) // initialize semaphores
    {
        rc = sem_init (&buf->compressed[i], 0, 0);
        rc |= sem_init (&buf->freed[i], 0, 1);
        exit_if (rc != 0, "cannot init semaphore");
    }

    /* create C threads */
    compressors = cpu_cores > 1? cpu_cores - 1: 1; // guard against single-core edge case
    pthread_t c_threads[compressors];
    C_arg_t cargs[compressors];
    for (int i = 0; i < compressors; i++)
    {
        cargs[i].id = i;
        rc = pthread_create (&c_threads[i], NULL, C, &cargs[i]);
        exit_if (rc != 0, "cannot create compressor thread");
    }

    /* create a W thread */
    pthread_t w;
    rc = pthread_create (&w, NULL, W, NULL);
    exit_if (rc != 0, "cannot create writer thread");

    /* wait for W to finish */
    pthread_join (w, (void **) &rc); // no need to join C threads

    /* cleaning */
    rc = munmap (file_start, file_size); // un-mmap the file
    exit_if (rc != 0, "cannot un-mmap input file");
    for (int i = 0; i < buf->n_slots; i++)
        free (buf->data[i]);  
    free (buf->data);
    free (buf->compressed);
    free (buf->freed);
    free (buf);
}

uint64_t get_file_size (int fd)
{
    struct stat stats;
    fstat (fd, &stats);
    return stats.st_size;
}

uint64_t get_total_ram ()
{
    struct sysinfo s;
    int rc = sysinfo(&s);
    exit_if(rc != 0, "cannot obtain sysinfo\n");
    return s.totalram;
}

void write_to_buf (uint32_t count, char ch, char* ptr)
{
    *(uint32_t *) ptr = count;
    ptr += sizeof (uint32_t);
    *ptr = ch;
}

void write_out (uint32_t count, char ch, FILE* out)
{
    #ifdef WRITE_TO_STD
        fwrite (&count, sizeof (uint32_t), 1, stdout);
        fwrite (&ch, sizeof (char), 1, stdout);
    #endif
    #ifdef WRITE_TO_FILE
        fwrite (&count, sizeof (uint32_t), 1, out);
        fwrite (&ch, sizeof (char), 1, out);
    #endif
}

void exit_if (int boolean, const char* msg)
{
    if (boolean) {
        fprintf (stderr, "%s", msg);
        exit (1);
    }
}
