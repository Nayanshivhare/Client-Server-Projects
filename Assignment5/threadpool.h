#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define DBG
#include "dbg.h"

// maximum number of threads allowed in a pool
#define THREADS_IN_POOL 30

// structure represent tasks
typedef struct task_st {
    void (*task_function_ptr) (void *);
    void *args;
    struct task_st* next;
} task_t;

// structure representing threadpools
typedef struct _threadpool_st {
    // task queueu
    int task_count;
    task_t *task_queueu_head;
    task_t *task_queueu_tail;

    // threads
    int thread_count;
    pthread_t *threads;

    // mutex and conditions
    pthread_mutex_t task_init_guard;
    pthread_mutex_t thread_pool_guard;
    pthread_cond_t task_available;
} _threadpool;

// type definition for threadpool
typedef void *threadpool;

// low-level thread function that executes job routines
// not to be used on its own and thus not part of api
void *_thread_function(threadpool my_thread_pool);

//type of function that copies arguments into thread space, see below
typedef void *(*task_copy_arguments_function_ptr)(void *);


/* **************************************************** */
/* PUBLIC API                                           */
/* **************************************************** */

// function that creates a threadpool
threadpool threadpool_create(void);

// this function needs to be implemented
// it copies the arguments of the function that is executed in a thread
// over to some dynamically allocated memory space and returns a pointer to it
// that way, spurious overrides cannot occur
void *task_copy_arguments(void *args);

// type of pointer to the function that is executed in a thread
typedef void (*task_function_ptr)(void *);

// putting a task into task queueu, where it is going to be executed by a thread
//void threadpool_add_task(threadpool my_thread_pool, task_prepare_args_function_ptr task_prepare_args_function, task_function_ptr task_function, void *arg);
void threadpool_add_task(threadpool my_thread_pool, task_copy_arguments_function_ptr task_copy_arguments_function, task_function_ptr task_function, void *arg);

// destroy threadpool
void threadpool_destroy(threadpool my_thread_pool);

