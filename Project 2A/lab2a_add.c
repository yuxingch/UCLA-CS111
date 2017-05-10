//
//  lab2_add.c
//  lab2a
//
//  Created by Yuxing Chen on 5/4/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>   //  clock_gettime()
#include <pthread.h>

#define BILLION     1000000000L
#define THREADS     't'
#define ITERATIONS  'i'
#define YIELD       'y'
#define SYNC        'z'
#define NO_METHOD   'n'
#define MUTEX       'm'
#define SPINLOCK    's'
#define COMPARESWAP 'c'


//  http://stackoverflow.com/questions/1856599/when-to-use-static-keyword-before-global-variables
static long long counter    = 0;   //  initializes a (long long) counter to zero
static int n_threads        = 1;  //  takes a parameter for the number of iterations (--iterations=#, default 1)
static int n_iterations     = 1;
static int opt_yield        = 0;
static char method          = NO_METHOD;
pthread_mutex_t mutex       = PTHREAD_MUTEX_INITIALIZER;
static int lock             = 0;


void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

//void add(long long *pointer, long long value) {
//    long long sum = *pointer + value;
//    *pointer = sum;
//}

/**
 *  starts the specified number of threads, each of which will use the above add function to
 *      add 1 to the counter the specified number of times 
 *      add -1 to the counter the specified number of times 
 *      exit to re-join the parent thread
 */
void* my_thread_add() {
    //  add 1 to the counter n_iterations times
    for (int i = 0; i < n_iterations; i++) {
        if (method == NO_METHOD) {  //  no sync mehtod specified
            add(&counter, 1);
        } else if (method == MUTEX){    //  mutex,
            pthread_mutex_lock(&mutex);
            add(&counter, 1);
            pthread_mutex_unlock(&mutex);
        } else if (method == SPINLOCK) {
            //  https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
            while(__sync_lock_test_and_set (&lock, 1))
                ;
            add(&counter, 1);
            __sync_lock_release (&lock);
        } else if (method == COMPARESWAP) {
            long long old, new;
            do {
                if (opt_yield)
                    sched_yield();
                old = counter;
                new = old + 1;
            } while ( __sync_val_compare_and_swap(&counter, old, new) != old);
//            old = counter;
//            new = old + 1;
//            while ( __sync_val_compare_and_swap(&counter, old, new) != old) {
//                if (opt_yield)
//                    sched_yield();
//                old = counter;
//                new = old + 1;
//            }
        }
    }
    for (int i = 0; i < n_iterations; i++) {
        if (method == NO_METHOD) {
            add(&counter, -1);
        } else if (method == MUTEX){
            pthread_mutex_lock(&mutex);
            add(&counter, -1);
            pthread_mutex_unlock(&mutex);
        } else if (method == SPINLOCK) {
            while(__sync_lock_test_and_set (&lock, 1))
                ;
            add(&counter, -1);
            __sync_lock_release (&lock);
        } else if (method == COMPARESWAP) {
            long long old, new;
            do {
                if (opt_yield)
                    sched_yield();
                old = counter;
                new = old - 1;
            } while ( __sync_val_compare_and_swap(&counter, old, new) != old);
//            old = counter;
//            new = old - 1;
//            while ( __sync_val_compare_and_swap(&counter, old, new) != old) {
//                if (opt_yield)
//                    sched_yield();
//                old = counter;
//                new = old - 1;
            /*  Missing yield() function    */
//            }
        }
    }
    return NULL;
}

int main(int argc, char * argv[]) {
    static struct option long_options[] = {
        {"threads",     required_argument,  0,  THREADS },
        {"iterations",  required_argument,  0,  ITERATIONS  },
        {"yield",       no_argument,        0,  YIELD   },
        {"sync",        required_argument,  0,  SYNC    },
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int opt;
    int long_index = 0;
    
    while ( (opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch (opt) {
            case THREADS:       n_threads = atoi(optarg);       break;
            case ITERATIONS:    n_iterations = atoi(optarg);    break;
            case YIELD:         opt_yield = 1;                  break;
            case SYNC:
                if (optarg[0] == MUTEX) {
                    method = MUTEX;
                } else if (optarg[0] == SPINLOCK) {
                    method = SPINLOCK;
                } else if (optarg[0] == COMPARESWAP) {
                    method = COMPARESWAP;
                } else {
                    fprintf(stderr, "Bad arguments are encountered. Exit with 1. \n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Bad arguments are encountered. Exit with 1. \n");
                exit(1);
                break;
        }
    }
    
    //  form the first argument of the output
    char* base = "add";
    char test_name[15];
    strcpy(test_name, base);
    if (opt_yield) {
        strcat(test_name, "-yield");
    }
    if (method != NO_METHOD) {
        char buf[2];
        sprintf(buf, "-%c", method);
        strcat(test_name, buf);
    } else {
        strcat(test_name, "-none");
    }
    
    
    /*	ROUTINE
     *	start_time
     *	pthread_create()
     *	in each thread
     *		for (i = 0; i < #it; i++)
     *		{add(&counter,1);}
     *		for (...)
     *		{......-1}
     *	end_time
     */
    
    //  notes the (high resolution) starting time for the run (using clock_gettime(3))
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    struct timespec starting_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &starting_time) < 0) {
        fprintf(stderr, "Failure in clock_gettime().\n");
        exit(1);
    }
    
    //  pthread_create
    //  Each of the threads in a process has a unique thread identifier (stored in the type pthread_t).
    //      This identifier is returned to the caller of pthread_create(3).
    pthread_t* thread_IDs = (pthread_t*) malloc(n_threads * sizeof(pthread_t));
    if (thread_IDs == NULL) {
        fprintf(stderr, "Failure in allocating space for threads.\n");
        exit(1);
    }
    
    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&thread_IDs[i], NULL, my_thread_add, NULL) != 0) {
            fprintf(stderr, "Failure in pthread_create.\n");
            free(thread_IDs);
            exit(1);
        }
    }
    
    //  pthread_join
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(thread_IDs[i], NULL) != 0) {
            fprintf(stderr, "Failure in pthread_join.\n");
            exit(1);
        }
    }
    
    //  end_time for the run
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
        fprintf(stderr, "Failure in clock_gettime().\n");
        exit(1);
    }
    
    //  calculate the time
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec) + (end_time.tv_nsec - starting_time.tv_nsec);
//    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec);
//    total_run_time += end_time.tv_nsec;
//    total_run_time -= starting_time.tv_nsec;
    long long ops = n_threads * n_iterations * 2;
    long long avg_run_time = total_run_time / ops;
    
    //  prints to stdout a comma-separated-value (CSV) record
    //  eg. add-none,10,10000,200000,6574000,32,374
    printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", test_name, n_threads, n_iterations, ops, total_run_time, avg_run_time, counter);
    //  free the allocated memory: http://stackoverflow.com/questions/24686182/when-to-free-dynamically-allocated-pthread-t-pointer-in-c
    free(thread_IDs);
    exit(0);
}
