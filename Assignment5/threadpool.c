#include "threadpool.h"

/* ************************************************************************ */
/* create a threadpool                                                      */
/* ************************************************************************ */

threadpool threadpool_create() {
    
    _threadpool *pool;
    int i;
    
    // ----------------------------------------------------------
    // prepare threadpool
    // ----------------------------------------------------------
    pool = (_threadpool *) malloc(sizeof(_threadpool));
    if (pool == NULL)
    {
        fprintf(stderr, "\n[threadpool_create] out of memory creating a new threadpool!");
        return NULL;
    }
    
    // ----------------------------------------------------------
    // prepare task queueu
    // ----------------------------------------------------------
    pool->task_count = 0;
    pool->task_queueu_head = NULL;
    pool->task_queueu_tail = NULL;

    // ----------------------------------------------------------
    // prepare threads
    // ----------------------------------------------------------
    pool->thread_count = THREADS_IN_POOL;
    pool->threads = (pthread_t *)malloc(THREADS_IN_POOL * sizeof(pthread_t));

    for (i=0; i< pool->thread_count ; i++)
    {
        if(pthread_create(&(pool->threads[i]), NULL, _thread_function, pool) == 0)
        {
            pthread_detach(pool->threads[i]);
        }
        else
        {
            fprintf(stderr, "\n[threadpool_create] error during threadpool creation!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // ----------------------------------------------------------
    // prepare mutex and condition variables
    // ----------------------------------------------------------
    pthread_mutex_init(&(pool->task_init_guard),    NULL);
    pthread_mutex_init(&(pool->thread_pool_guard),  NULL);
    pthread_cond_init( &(pool->task_available),     NULL);
    
    debug("[create_threadpool] threadpool with %d threads created\n", i);
    
    return (threadpool) pool;
}


/* ************************************************************************ */
/* add a task to the task queueu                                            */
/* ************************************************************************ */

void threadpool_add_task(
             threadpool my_thread_pool,
             task_copy_arguments_function_ptr task_copy_arguments,
             task_function_ptr task_function,
             void *arg) {
    
    _threadpool *pool = (_threadpool *) my_thread_pool;
    task_t *current_task;
 

    // ----------------------------------------------------------
    // about the prepare a task, lock task initialization
    // ----------------------------------------------------------
    pthread_mutex_lock(&(((_threadpool *)pool)->task_init_guard));


    // ----------------------------------------------------------
    // prepare a task to be executed
    // ----------------------------------------------------------
    current_task = (task_t *)malloc(sizeof(task_t));
    if (current_task == NULL)
    {
        fprintf(stderr, "Error allocating space for task\n");
        return;
    }
    
    // load function pointer and args into task structure
    current_task->task_function_ptr = task_function;
    current_task->args = (task_copy_arguments)(arg);
    current_task->next = NULL;
    
    
    // ----------------------------------------------------------
    // we are done initializing task, unlock task initialization
    // ----------------------------------------------------------
    pthread_mutex_unlock(&(((_threadpool *)pool)->task_init_guard));

    
    // ----------------------------------------------------------
    // lock the threadpool
    // ----------------------------------------------------------
    pthread_mutex_lock(&(pool->thread_pool_guard));
            

    // ----------------------------------------------------------
    // put task into task queueu
    // ----------------------------------------------------------
    if (pool->task_count == 0)
    {
        // task queueu was empty, new task is both head/tail of queueu
        pool->task_queueu_head = current_task;
        pool->task_queueu_tail = current_task;
    }
    else
    {
        // add new task at the end of task queueu
        pool->task_queueu_tail->next = current_task;
        pool->task_queueu_tail = current_task;
    }
    
    // update number of tasks in task queueu
    pool->task_count++;
    
    
    // ----------------------------------------------------------
    // let threads know that a task is available
    // ----------------------------------------------------------
    pthread_cond_signal(&(pool->task_available));

    
    // ----------------------------------------------------------
    // unlock threadpool
    // ----------------------------------------------------------
    pthread_mutex_unlock(&(pool->thread_pool_guard));
}


/* ************************************************************************ */
/* generic function executed within a thread that executes a job_routine    */
/* NOT intended to be used on its own                                       */
/* ************************************************************************ */

void *_thread_function(threadpool my_thread_pool)
{
    _threadpool* pool = (_threadpool *) my_thread_pool;
    task_t *current_task;
    
    // ----------------------------------------------------------
    // execution loop of thread
    // ----------------------------------------------------------
    while (true)
    {
        // ----------------------------------------------------------
        // lock threadpool
        // ----------------------------------------------------------
        pthread_mutex_lock(&(pool->thread_pool_guard));

        
        // ----------------------------------------------------------
        // if there is no task available, wait for one coming in
        // here "spurious wakeups" may occur, hence the while, not if
        // ----------------------------------------------------------
        while(pool->task_count == 0)
        {
            pthread_cond_wait(&(pool->task_available), &(pool->thread_pool_guard));
        }

        
        // ----------------------------------------------------------
        // get the task to be executed from head of task queueu
        // ----------------------------------------------------------
        current_task = pool->task_queueu_head;

        pool->task_count--;
        if (pool->task_count == 0)
        {
            pool->task_queueu_head = NULL;
            pool->task_queueu_tail = NULL;
        }
        else
        {
            pool->task_queueu_head = current_task->next;
        }
       
        
        // ----------------------------------------------------------
        // one thread less available
        // ----------------------------------------------------------
        pool->thread_count--;
        
        
        // ----------------------------------------------------------
        // unlock threadpool
        // ----------------------------------------------------------
        pthread_mutex_unlock(&(pool->thread_pool_guard));

        
        // ----------------------------------------------------------
        // execute task - this is THE BIG THING!
        // ----------------------------------------------------------
        (current_task->task_function_ptr) (current_task->args);

        
        // ----------------------------------------------------------
        // returned executing task, make thread available again
        // ----------------------------------------------------------
        pthread_mutex_lock(&(pool->thread_pool_guard));
        // increase thread count
        pool->thread_count++;
        pthread_mutex_unlock(&(pool->thread_pool_guard));

        
        // ----------------------------------------------------------
        // free space claimed by task ...
        // ----------------------------------------------------------
        free(current_task);
        
        // ----------------------------------------------------------
        //  and start all over ...
        // ----------------------------------------------------------
    }
}


/* ************************************************************************ */
/* destroy threadpool                                                       */
/* ************************************************************************ */

void threadpool_destroy(threadpool my_thread_pool) {
    _threadpool *pool = (_threadpool *) my_thread_pool;
    
    pthread_mutex_destroy(&(pool->task_init_guard));
    pthread_mutex_destroy(&(pool->thread_pool_guard));
    pthread_cond_destroy (&(pool->task_available));

    pool->thread_count = 0;
 
    free(pool->threads);
    free(pool);
 
    return; 
}

