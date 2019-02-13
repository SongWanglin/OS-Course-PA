#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#define SAMPLES 1000
#define BASE_10 10

void time_array(struct timeval times[SAMPLES]);
double avg_syscall_cost(int samples, size_t nbyte);
double avg_syscall_cost_fopen(int samples, size_t nbyte);
double avg_syscall_cost_2(int samples);

int main(int argc, char *argv[])
{
    /* if the user provides an integer use that as the # of samples */
    long usr_samples = argc > 1 ? strtol(argv[1], NULL, BASE_10) : 0;
    usr_samples = (usr_samples ? usr_samples : SAMPLES);

    /* perform experiments */
    printf("The average time for a system call (read, 0-byte) is %.3fms\n", 
            avg_syscall_cost(usr_samples, 0));
    printf("The average time for a system call (read, 1-byte) is %.3fms\n", 
            avg_syscall_cost(usr_samples, 1));
    printf("The average time for a system call (fread, 0-byte) is %.3fms\n",
            avg_syscall_cost_fopen(usr_samples, 0));
    printf("The average time for a system call (fread, 1-byte) is %.3fms\n",
            avg_syscall_cost_fopen(usr_samples, 1));
    printf("The average time for a system call is %.3fms\n",
            avg_syscall_cost_2(usr_samples));
    return 0;
}

/*
 * for an interesting implementation of cross OS compatible real-time benchmarking:
 * http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking
 */

/*
 * Measure the costs of a system call, by repeatedly performing a nbyte-byte read.
 * @param samples, the number of measurements taken
 * @param nbyte, the number of bytes to read
 * @return the average time to perform the syscall in microseconds.
 */
double avg_syscall_cost(int samples, size_t nbyte)
{
    struct timeval start, end;
    suseconds_t diff;
    unsigned long ms_total = 0;
    int tod1, tod2, file;
    char buf;

    for (int i=0; i < samples; i++) {
        /* open the file */
        file = open("blank_file.output", O_RDONLY|O_CREAT);
        if (file == -1) {
            fprintf(stderr, "file error\n");
            exit(EXIT_FAILURE);
        }

        /* benchmark */
        tod1 = gettimeofday(&start, NULL);  // start time
        read(file, &buf, nbyte);
        tod2 = gettimeofday(&end, NULL);    // end time
        
        if (tod1 == -1 || tod2 == -1) {
            fprintf(stderr, "failed benchmarking time\n");
            exit(EXIT_FAILURE);
        }

        diff = end.tv_usec - start.tv_usec;
        ms_total += (int) diff;

        /* close the file */
        close(file);
    }
    return (double) ms_total / (double) samples;
}

/*
 * Measure the costs of a system call, by repeatedly performing a nbyte-byte fread.
 * @param samples, the number of measurements taken
 * @param nbyte, the number of bytes to read
 * @return the average time to perform the syscall in microseconds.
 */
double avg_syscall_cost_fopen(int samples, size_t nbyte)
{
    struct timeval start, end;
    char c;
    suseconds_t diff;
    unsigned long ms_total = 0;
    int tod1, tod2;
    FILE *file;

    for (int i=0; i < samples; i++) {
        /* open the file */
        file = fopen("blank_file.output", "w+");
        if (file == NULL) {
            fprintf(stderr, "file error\n");
            exit(EXIT_FAILURE);
        }

        /* benchmark */
        tod1 = gettimeofday(&start, NULL);  // start time
        fread(&c, 1, nbyte, file);              // perform syscall
        tod2 = gettimeofday(&end, NULL);    // end time
        
        if (tod1 == -1 || tod2 == -1) {
            fprintf(stderr, "failed benchmarking time\n");
            exit(EXIT_FAILURE);
        }

        diff = end.tv_usec - start.tv_usec;
        ms_total += (int) diff;

        /* close the file */
        fclose(file);
    }
    return (double) ms_total / (double) samples;
}

/*
 * Measure the costs of a system call, by repeatedly performing a 0-byte read.
 * @param samples, the number of measurements taken
 * @return the average time to perform the syscall in microseconds.  
 */
double avg_syscall_cost_2(int samples)
{
    struct timeval start, end;
    char c;
    suseconds_t diff;
    unsigned long ms_total = 0;
    int tod1, tod2;
    FILE *file;
    
    /* open the file */
    file = fopen("blank_file.output", "w+");
    if (file == NULL) {
        fprintf(stderr, "file error\n");
        exit(EXIT_FAILURE);
    }

    for (int i=0; i < samples; i++) {
        /* benchmark */
        tod1 = gettimeofday(&start, NULL);  // start time
        fread(&c, 1, 1, file);              // perform syscall
        tod2 = gettimeofday(&end, NULL);    // end time
        
        if (tod1 == -1 || tod2 == -1) {
            fprintf(stderr, "failed benchmarking time\n");
            exit(EXIT_FAILURE);
        }

        diff = end.tv_usec - start.tv_usec;
        ms_total += (int) diff;
    }
    /* close the file */
    fclose(file);

    return (double) ms_total / (double) samples;
}

/*
 * QUESTIONS:
 * 1. a) Do the fopen(), fread(), etc, procedures call open(), read() (etc) or implement their own syscalls?
 *    b) Presumably the former exist as syntactic sugar, and as a cross OS implementation?
 *    c) Why such a discrepency in between read() and fread()?
 * 3. Is there an OSX equivalent to Linux's sched_setaffinity()?
 * 4. If not, will the above procedure work in a VM? 
 */
