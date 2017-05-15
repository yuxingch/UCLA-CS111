//
//  lab2_list.c
//  lab2b
//
//  Created by Yuxing Chen on 5/11/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <time.h>   //  clock_gettime()
#include <pthread.h>
#include <signal.h>

#include "SortedList.h"

#define BILLION     1000000000L
#define THREADS     't'
#define ITERATIONS  'i'
#define YIELD       'y'
#define SYNC        'z'
#define LISTS       'l'
#define NO_METHOD   'n'
#define MUTEX       'm'
#define SPINLOCK    's'

#define ID_YIELD    0x03
#define IL_YIELD    0x05
#define DL_YIELD    0x06
#define IDL_YIELD   0x07

static int n_threads        = 1;  //  takes a parameter for the number of iterations (--iterations=#, default 1)
static int n_iterations     = 1;
static long long n_elements = 0;
int opt_yield               = 0;
static char method          = NO_METHOD;
//pthread_mutex_t mutex       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t* mutex      = NULL;
int* spinlocks              = NULL;

static int key_length       = 5;
char key_candidate[53]      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int n_lists          = 1; // Since by default, there should be one (big) list.
static long long* elapse_diff = NULL;

SortedList_t** list_heads = NULL;
SortedListElement_t* element_list = NULL;

//  http://stackoverflow.com/questions/7666509/hash-function-for-string
//  http://www.cse.yorku.ca/~oz/hash.html
unsigned int select_list(const char* key) {
    unsigned long hash = 5381;
    for (int i = 0; i < key_length; i++)
        hash = ((hash << 5) + hash) + key[i];   /* hash * 33 + c */
    unsigned int ret = hash % n_lists;
    return ret;
}

void free_all_lists(int bound) {
    for (int i = 0; i < bound; i++) {
        free(list_heads[i]);
    }
    free(list_heads);
}

void free_locks() {
    if (method == MUTEX) {
        free(mutex);
        free(elapse_diff);
    } else if (method == SPINLOCK) {
        free(spinlocks);
    }
}

//char* random_key() {
//    //  http://stackoverflow.com/questions/33464816/c-generate-random-string-of-max-length
//    srand((unsigned int)time(NULL));
//    char* my_key = malloc((key_length + 1) * (sizeof(char)));
//    for (int i = 0; i < key_length; i++) {
//        my_key[i] = key_candidate[rand() % 52];
//    }
//    my_key[key_length] = '\0';
//    return my_key;
//}

void elem_init() {
//    for (int i = 0; i < n_elements; i++)
//        element_list[i].key = random_key();
    srand((unsigned int) time(NULL));
    for (unsigned int i = 0; i < n_elements; i++) {
        char* my_key = malloc((key_length + 1) * (sizeof(char)));
        for (int i = 0; i < key_length; i++) {
            my_key[i] = key_candidate[rand() % 52];
        }
        my_key[key_length] = '\0';
        element_list[i].key = my_key;
    }
}

/******************
 *  each thread:
 *      starts with a set of pre-allocated and initialized elements (--iterations=#)
 *      inserts them all into a (single shared-by-all-threads) list
 *      gets the list length
 *      looks up and deletes each of the keys it had previously inserted
 *      exits to re-join the parent thread
 ******************
 *  New features:
 *      Note the high-resolution time before and after getting the lock, 
 *      compute the elapsed difference, and add that to a (per-thread) total.
 */

void* my_thread_list (void* arg) {
    //  variables to collect time
    struct timespec start_lock, end_lock;
    //  use the argument passed into the thread function to get the starting position
    int pos = *((int*) arg);
    int curr_thread_id = pos / n_iterations;
    SortedList_t* sublist = NULL;
    
    //  inserts them all into a (single shared-by-all-threads) list
    for (int i = pos; i < pos + n_iterations; i++) {
        const char* curr_key = element_list[i].key;
        int sublist_id = select_list(curr_key);
        sublist = list_heads[sublist_id];
        if (method == NO_METHOD) {
            SortedList_insert(sublist, &element_list[i]);
        } else if (method == MUTEX){
            if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            //  get the lock
            pthread_mutex_lock(&mutex[sublist_id]);
            if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            //  compute the time difference (wait for lock)
            elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);
            //  insertion
            SortedList_insert(sublist, &element_list[i]);
            pthread_mutex_unlock(&mutex[sublist_id]);
        } else if (method == SPINLOCK) {
            if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            while(__sync_lock_test_and_set (&spinlocks[sublist_id], 1))
                ;
            if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            //  compute the time difference (wait for lock)
            elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);

            SortedList_insert(sublist, &element_list[i]);
            __sync_lock_release (&spinlocks[sublist_id]);
        }
    }
    //  gets the list length
    int list_len   = 0;
    if (method == NO_METHOD) {
        for (int i = 0; i < n_lists; i++) {
            int temp_len = SortedList_length(list_heads[i]);
            if (temp_len < 0) {
                fprintf(stderr, "Error: list length < 0. list is corrupted. Exit with 2.\n");
                exit(2);
            }
            list_len += temp_len;
        }
    } else if (method == MUTEX){
        if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
            fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
            exit(1);
        }
        //  get all locks
        for (int i = 0; i < n_lists; i++)
            pthread_mutex_lock(&mutex[i]);
        if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
            fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
            exit(1);
        }
        elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);
        //  protected region
        for (int i = 0; i < n_lists; i++) {
            int temp_len = SortedList_length(list_heads[i]);
            if (temp_len < 0) {
                fprintf(stderr, "Error: list length < 0. list is corrupted. Exit with 2.\n");
                exit(2);
            }
            list_len += temp_len;
        }
        //  unlock all
        for (int i = 0; i < n_lists; i++)
            pthread_mutex_unlock(&mutex[i]);
    } else if (method == SPINLOCK) {
        if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
            fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
            exit(1);
        }
        for (int i = 0; i < n_lists; i++) {
            while(__sync_lock_test_and_set (&spinlocks[i], 1))
                ;
        }
        if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
            fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
            exit(1);
        }
        //  compute the time difference (wait for lock)
        elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);

        for (int i = 0; i < n_lists; i++) {
            int temp_len = SortedList_length(list_heads[i]);
            if (temp_len < 0) {
                fprintf(stderr, "Error: list length < 0. list is corrupted. Exit with 2.\n");
                exit(2);
            }
            list_len += temp_len;
        }
        for (int i = 0; i < n_lists; i++)
            __sync_lock_release (&spinlocks[i]);
    }
    
    //  looks up and deletes each of the keys it had previously inserted
    for (int i = pos; i < pos + n_iterations; i++) {
        const char* curr_key = element_list[i].key;
        int sublist_id = select_list(curr_key);
        sublist = list_heads[sublist_id];
        if (method == NO_METHOD) {
            SortedListElement_t *temp = SortedList_lookup(sublist, curr_key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element. Exit with 2.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted. Exit with 2.\n");
                exit(2);
            }
        } else if (method == MUTEX){
            if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            //  get the lock
            pthread_mutex_lock(&mutex[sublist_id]);
            if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);
            
            SortedListElement_t *temp = SortedList_lookup(sublist, curr_key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element. Exit with 2.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted. Exit with 2.\n");
                exit(2);
            }
            pthread_mutex_unlock(&mutex[sublist_id]);
        } else if (method == SPINLOCK) {
            if (clock_gettime(CLOCK_MONOTONIC, &start_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            while(__sync_lock_test_and_set (&spinlocks[sublist_id], 1))
                ;
            if (clock_gettime(CLOCK_MONOTONIC, &end_lock) < 0) {
                fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
                exit(1);
            }
            elapse_diff[curr_thread_id] += BILLION * (end_lock.tv_sec - start_lock.tv_sec) + (end_lock.tv_nsec - start_lock.tv_nsec);
            SortedListElement_t *temp = SortedList_lookup(sublist, curr_key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element. Exit with 2.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted. Exit with 2.\n");
                exit(2);
            }
            __sync_lock_release (&spinlocks[sublist_id]);
        }
    }
    return NULL;
}

//  Segmentation fault handler
void my_handler(int signum) {
    if (signum == SIGSEGV) {
        fprintf(stderr, "Error: Segmentation Fault. Exit with 2.\n");
        exit(2);
    }
}

int main(int argc, char * argv[]) {
    static struct option long_options[] = {
        {"threads",     required_argument,  0,  THREADS },
        {"iterations",  required_argument,  0,  ITERATIONS  },
        {"yield",       required_argument,  0,  YIELD   },
        {"sync",        required_argument,  0,  SYNC    },
        {"lists",       required_argument,  0,  LISTS   },
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int opt;
    int long_index = 0;
    
    signal(SIGSEGV, my_handler);    //  signal for segmentation fault
    
    while ( (opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch (opt) {
            case THREADS:
                n_threads = atoi(optarg);
                break;
            case ITERATIONS:
                n_iterations = atoi(optarg);
                break;
            case YIELD:
                if (strlen(optarg) > 3) {
                    fprintf(stderr, "Error: Bad arguments are encountered. Exit with 1. \n");
                    exit(1);
                }
                for (int i = 0; i < strlen(optarg); i++) {
                    switch (optarg[i]) {
                        case 'i':   opt_yield |= INSERT_YIELD;  break;
                        case 'd':   opt_yield |= DELETE_YIELD;  break;
                        case 'l':   opt_yield |= LOOKUP_YIELD;  break;
                        default:
                            fprintf(stderr, "Error: Bad arguments are encountered. Exit with 1. \n");
                            exit(1);
                            break;
                    }
                }
                break;
            case SYNC:
                if (optarg[0] == MUTEX) {
                    method = MUTEX;
                } else if (optarg[0] == SPINLOCK) {
                    method = SPINLOCK;
                } else {
                    fprintf(stderr, "Error: Bad arguments are encountered. Exit with 1. \n");
                    exit(1);
                }
                break;
            case LISTS:
                n_lists = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Error: Bad arguments are encountered. Exit with 1. \n");
                exit(1);
                break;
        }
    }
    
    
    //  Initialize the list of the heads of sublists
//    if (n_lists > 1) {
    if ((list_heads = malloc(sizeof(SortedList_t*) * n_lists)) == NULL) {
        fprintf(stderr, "Error: failure in allocating space for list_heads. Exit with 1.\n");
        exit(1);
    }
    for (int i = 0; i < n_lists; i++) {
        if ((list_heads[i] = malloc(sizeof(SortedList_t))) == NULL) {
            free_all_lists(i-1);
            fprintf(stderr, "Error: failure in allocating space for list_heads. Exit with 1.\n");
            exit(1);
        }
        SortedList_t* hd = list_heads[i];
        list_heads[i]->prev = hd;
        list_heads[i]->next = hd;
        list_heads[i]->key = NULL;
    }
//    }
    
    //  Initialize the array of locks for each list
    if (method == MUTEX) {
        if ((elapse_diff = malloc(n_threads * sizeof(long long))) == NULL) {
            fprintf(stderr, "Error: failure in allocating space for elapse_diff. Exit with 1.\n");
            free_all_lists(n_lists);
            exit(1);
        }
        int i = 0;
        while (i < n_threads) {
            elapse_diff[i] = 0;
            i++;
        }
        if ((mutex = malloc(sizeof(pthread_mutex_t) * n_lists)) == NULL) {
            fprintf(stderr, "Error: failure in allocating space for mutex locks. Exit with 1.\n");
            free_all_lists(n_lists);
            free(elapse_diff);
            exit(1);
        }
        for (int i = 0; i < n_lists; i++) {
            pthread_mutex_init( &mutex[i], NULL );
        }
    } else if (method == SPINLOCK) {
        if ((elapse_diff = malloc(n_threads * sizeof(long long))) == NULL) {
            fprintf(stderr, "Error: failure in allocating space for elapse_diff. Exit with 1.\n");
            free_all_lists(n_lists);
            exit(1);
        }
        int i = 0;
        while (i < n_threads) {
            elapse_diff[i] = 0;
            i++;
        }
        if ((spinlocks = malloc(sizeof(int) * n_lists)) == NULL) {
            fprintf(stderr, "Error: failure in allocating space for spinlocks. Exit with 1.\n");
            free_all_lists(n_lists);
            exit(1);
        }
        for (int i = 0; i < n_lists; i++) {
            spinlocks[i] = 0;
        }
    }
    
    //  creates and initializes (with random keys) the required number (threads x iterations) of list elements.
    n_elements = n_threads * n_iterations;
    element_list = malloc(n_elements*(sizeof(SortedListElement_t)));
    elem_init();
    
    //  form the first argument of the output
    char* base = "list";
    char test_name[15]; //  list-none-none
    strcpy(test_name, base);
    switch (opt_yield) {
        case INSERT_YIELD:  strcat(test_name, "-i");    break;
        case DELETE_YIELD:  strcat(test_name, "-d");    break;
        case LOOKUP_YIELD:  strcat(test_name, "-l");    break;
        case ID_YIELD:      strcat(test_name, "-id");   break;
        case IL_YIELD:      strcat(test_name, "-il");   break;
        case DL_YIELD:      strcat(test_name, "-dl");   break;
        case IDL_YIELD:     strcat(test_name, "-idl");  break;
        default:            strcat(test_name, "-none"); break;
    }
    if (method != NO_METHOD) {
        char buf[2];
        sprintf(buf, "-%c", method);
        strcat(test_name, buf);
    } else {
        strcat(test_name, "-none");
    }
    
    //  create the argument for each thread, as a list, indicating the starting position of each thread.
    int args_list[n_threads];
    for (int i = 0; i < n_threads; i++) {
        args_list[i] = i * n_iterations;
    }
    
    //  notes the (high resolution) starting time for the run (using clock_gettime(3))
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    struct timespec starting_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &starting_time) < 0) {
        fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
        free(element_list);
        free_all_lists(n_lists);
        free_locks();
        exit(1);
    }
    
    //  pthread_create
    //  Each of the threads in a process has a unique thread identifier (stored in the type pthread_t).
    //      This identifier is returned to the caller of pthread_create(3).
    pthread_t* thread_IDs = (pthread_t*) malloc(n_threads * sizeof(pthread_t));
    if (thread_IDs == NULL) {
        fprintf(stderr, "Error: Failure in allocating space for threads. Exit with 1.\n");
        free(element_list);
        free_all_lists(n_lists);
        free_locks();
        exit(1);
    }
    
    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&thread_IDs[i], NULL, my_thread_list, (void *) &args_list[i]) != 0) {
            fprintf(stderr, "Error: Failure in pthread_create. Exit with 1.\n");
            free(thread_IDs);
            free(element_list);
            free_all_lists(n_lists);
            free_locks();
            exit(1);
        }
    }
    
    //  pthread_join
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(thread_IDs[i], NULL) != 0) {
            fprintf(stderr, "Error: Failure in pthread_join. Exit with 1.\n");
            free(thread_IDs);
            free(element_list);
            free_all_lists(n_lists);
            free_locks();
            exit(1);
        }
    }
    
    //  end_time
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
        fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
        free(thread_IDs);
        free(element_list);
        free_all_lists(n_lists);
        free_locks();
        exit(1);
    }
    
    //  checks the length of the list to confirm that it is zero
//    int list_length = SortedList_length(my_list);
    int list_length = 0;
    int t, curr_len;
    for (t = 0, curr_len = 0; t < n_lists; t++) {
        curr_len = SortedList_length(list_heads[t]);
        if (curr_len < 0) {
            fprintf(stderr, "Error: list is corrupted. Exit with 2.\n");
            free(thread_IDs);
            free(element_list);
            free_all_lists(n_lists);
            free_locks();
            exit(2);
        }
        list_length += curr_len;
    }
    
    if (list_length != 0) {
        fprintf(stderr, "Error: The length of the list is not zero. Length is %d. Exit with 2.\n", list_length);
        free(thread_IDs);
        free(element_list);
        free_all_lists(n_lists);
        free_locks();
        exit(2);
    }
    
    //  calculate the time
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec) + (end_time.tv_nsec - starting_time.tv_nsec);
    //    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec);
    //    total_run_time += end_time.tv_nsec;
    //    total_run_time -= starting_time.tv_nsec;
    long long ops = n_threads * n_iterations * 3;
    long long avg_run_time = total_run_time / ops;
    
    long long wait_for_lock_time = 0;
    long long avg_wait_for_lock_time = 0;
    if (method == MUTEX || method == SPINLOCK) {
        for (int i = 0; i < n_threads; i++) {
            wait_for_lock_time += elapse_diff[i];
        }
        long long lock_ops = n_threads * (n_iterations * 2 + 1);
        avg_wait_for_lock_time = wait_for_lock_time / lock_ops;
    }
    //  print
    //
    printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", test_name, n_threads, n_iterations, n_lists, ops, total_run_time, avg_run_time, avg_wait_for_lock_time);
    //  free the allocated memory: http://stackoverflow.com/questions/24686182/when-to-free-dynamically-allocated-pthread-t-pointer-in-c
    free(thread_IDs);
    free(element_list);
    free_all_lists(n_lists);
    free_locks();
    exit(0);
}


