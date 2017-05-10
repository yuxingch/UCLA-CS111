//
//  lab2_list.c
//  lab2a
//
//  Created by Yuxing Chen on 5/4/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <time.h>   //  clock_gettime()
#include <pthread.h>

#include "SortedList.h"

#define BILLION     1000000000L
#define THREADS     't'
#define ITERATIONS  'i'
#define YIELD       'y'
#define SYNC        'z'
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
pthread_mutex_t mutex       = PTHREAD_MUTEX_INITIALIZER;
static int lock             = 0;
static int key_length       = 5;
char key_candidate[53]      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

SortedList_t* my_list = NULL;
SortedListElement_t* element_list = NULL;
//static int* args_list = NULL;

char* random_key() {
    //  http://stackoverflow.com/questions/33464816/c-generate-random-string-of-max-length
    srand((unsigned int)time(NULL));
    char* my_key = malloc((key_length + 1) * (sizeof(char)));
    for (int i = 0; i < key_length; i++) {
        my_key[i] = key_candidate[rand() % 52];
    }
    my_key[key_length] = '\0';
    return my_key;
}

void elem_init() {
    for (int i = 0; i < n_elements; i++)
        element_list[i].key = random_key();
}

//void args_init() {
//    args_list = malloc(n_threads * (sizeof(int)));
//    int i;
//    int temp = 0;
//    while (i < n_threads) {
//        args_list[i] = temp;
//        i++;
//        temp += n_iterations;
//    }
//}

void* my_thread_list (void* arg) {
    int pos = *((int*) arg);
    //  inserts them all into a (single shared-by-all-threads) list
    for (int i = pos; i < pos + n_iterations; i++) {
        if (method == NO_METHOD) {
            SortedList_insert(my_list, &element_list[i]);
        } else if (method == MUTEX){
            pthread_mutex_lock(&mutex);
            //  protected region
            SortedList_insert(my_list, &element_list[i]);
            pthread_mutex_unlock(&mutex);
        } else if (method == SPINLOCK) {
            //  https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
            while(__sync_lock_test_and_set (&lock, 1))
                ;
            SortedList_insert(my_list, &element_list[i]);
            __sync_lock_release (&lock);
        }
    }
    //  gets the list length
    int list_len   = 0;
    if (method == NO_METHOD) {
        list_len = SortedList_length(my_list);
    } else if (method == MUTEX){
        pthread_mutex_lock(&mutex);
        //  protected region
        list_len = SortedList_length(my_list);
        pthread_mutex_unlock(&mutex);
    } else if (method == SPINLOCK) {
        //  https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
        while(__sync_lock_test_and_set (&lock, 1))
            ;
        list_len = SortedList_length(my_list);
        __sync_lock_release (&lock);
    }
    
    //  TODO: list length for what??
    if (list_len < 0) {
        fprintf(stderr, "Error: list length < 0. list is corrupted.\n");
        exit(1);
    }
    
    //  looks up and deletes each of the keys it had previously inserted
    for (int i = pos; i < pos + n_iterations; i++) {
        if (method == NO_METHOD) {
            SortedListElement_t *temp = SortedList_lookup(my_list, element_list[i].key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted.\n");
                exit(2);
            }
        } else if (method == MUTEX){
            pthread_mutex_lock(&mutex);
            SortedListElement_t *temp = SortedList_lookup(my_list, element_list[i].key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted.\n");
                exit(2);
            }
            pthread_mutex_unlock(&mutex);
        } else if (method == SPINLOCK) {
            //  https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
            while(__sync_lock_test_and_set (&lock, 1))
                ;
            SortedListElement_t *temp = SortedList_lookup(my_list, element_list[i].key);
            if (temp == NULL) {
                fprintf(stderr, "Error: fail to look up the element.\n");
                exit(2);
            }
            if (SortedList_delete(temp)) {
                fprintf(stderr, "Error in delete. list is corrupted.\n");
                exit(2);
            }
            __sync_lock_release (&lock);
        }
    }
    
    return NULL;
}

int main(int argc, char * argv[]) {
    static struct option long_options[] = {
        {"threads",     required_argument,  0,  THREADS },
        {"iterations",  required_argument,  0,  ITERATIONS  },
        {"yield",       required_argument,  0,  YIELD   },
        {"sync",        required_argument,  0,  SYNC    },
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int opt;
    int long_index = 0;
    
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
            default:
                fprintf(stderr, "Error: Bad arguments are encountered. Exit with 1. \n");
                exit(1);
                break;
        }
    }
    
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
    
    //  Initialize our list (where we do the insertion)
    my_list= malloc(sizeof(SortedList_t));
    my_list->prev = my_list;
    my_list->next = my_list;
    my_list->key = NULL;
    
    //  creates and initializes (with random keys) the required number (threads x iterations) of list elements.
    /*  create  */
    n_elements = n_threads * n_iterations;
    element_list = malloc(n_elements*(sizeof(SortedListElement_t)));
    
    /*  initialization  */
    elem_init();
//    args_init();
    
    //  create the argument for each thread, as a list, indicating the starting position of each thread.
    int args_list[n_threads];
    for (int i = 0; i < n_threads; i++) {
        args_list[i] = i * n_iterations;
    }
    
    //  notes the (high resolution) starting time for the run (using clock_gettime(3))
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    struct timespec starting_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &starting_time) < 0) {
        fprintf(stderr, "Error: Failure in clock_gettime().\n");
        exit(1);
    }
    
    //  pthread_create
    //  Each of the threads in a process has a unique thread identifier (stored in the type pthread_t).
    //      This identifier is returned to the caller of pthread_create(3).
    pthread_t* thread_IDs = (pthread_t*) malloc(n_threads * sizeof(pthread_t));
    if (thread_IDs == NULL) {
        fprintf(stderr, "Error: Failure in allocating space for threads.\n");
        free(element_list);
        free(my_list);
//        free(args_list);
        exit(1);
    }
    
    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&thread_IDs[i], NULL, my_thread_list, (void *) &args_list[i]) != 0) {
            fprintf(stderr, "Error: Failure in pthread_create.\n");
            free(thread_IDs);
            free(element_list);
            free(my_list);
//            free(args_list);
            exit(1);
        }
    }
    
    //  pthread_join
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(thread_IDs[i], NULL) != 0) {
            fprintf(stderr, "Error: Failure in pthread_join.\n");
            free(thread_IDs);
            free(element_list);
            free(my_list);
//            free(args_list);
            exit(1);
        }
    }
    
    //  end_time
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
        fprintf(stderr, "Error: Failure in clock_gettime().\n");
        free(thread_IDs);
        free(element_list);
//        free(args_list);
        free(my_list);
        exit(1);
    }
    
    //  calculate the time
    //  https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec) + (end_time.tv_nsec - starting_time.tv_nsec);
    //    long long total_run_time = BILLION * (end_time.tv_sec - starting_time.tv_sec);
    //    total_run_time += end_time.tv_nsec;
    //    total_run_time -= starting_time.tv_nsec;
    long long ops = n_threads * n_iterations * 3;
    long long avg_run_time = total_run_time / ops;
    
    //  Check final list length
    int listlen = SortedList_length(my_list);
    if (listlen != 0) {
        if (listlen > 0)
            fprintf(stderr, "Final list length is not zero, count: %d\n", listlen);
        else
            fprintf(stderr, "Error: list is corrupted.\n");
        free(thread_IDs);
        free(element_list);
        free(my_list);
//        free(args_list);
        exit(2);
    }
    
    int n_list = 1;
    //  print
    //  add-none,10,10000,200000,6574000,32,374
    printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, n_threads, n_iterations, n_list, ops, total_run_time, avg_run_time);
    //  free the allocated memory: http://stackoverflow.com/questions/24686182/when-to-free-dynamically-allocated-pthread-t-pointer-in-c
    free(thread_IDs);
    free(element_list);
    free(my_list);
//    free(args_list);
    exit(0);
}

