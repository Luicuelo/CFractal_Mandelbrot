/*
 * Copyright (c) 2016, Mathias Brossard <mathias@brossard.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */

#include <stdlib.h>
#include "threadpool.h"
#include "utils.h"



typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct {
    void (*function)(void *);
    RenderFractalInternalParams argument; // direct argument ,no malloc/free
} threadpool_task_t;

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
struct threadpool_t {
  mtx_t lock;
  cnd_t notify;
  cnd_t queue_notify;
  cnd_t all_tasks_done;
  thrd_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
};

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;
    int i;

    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
        return NULL;
    }

    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;
  

    /* Allocate thread and task queue */
    pool->threads = (thrd_t *)malloc(sizeof(thrd_t) * thread_count);
    pool->queue = (threadpool_task_t *)malloc
        (sizeof(threadpool_task_t) * queue_size);

    /* Initialize mutex and conditional variable first */

    
    if(mtx_init(&(pool->lock), mtx_plain)!= thrd_success) { goto err;}
    if(cnd_init(&(pool->notify)) != thrd_success) { goto err;}
    if(cnd_init(&(pool->queue_notify)) != thrd_success)  { goto err;}
    if(cnd_init(&(pool->all_tasks_done)) != thrd_success) { goto err;}
    if(pool->threads == NULL)  { goto err;}
    if(pool->queue == NULL) { goto err;}
   

    /* Start worker threads */
    for(i = 0; i < thread_count; i++) {
        if(thrd_create(&(pool->threads[i]), threadpool_thread, (void*)pool) != thrd_success) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *),
                  void* argument)
{
    int err = 0;
    int next;
    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    if(mtx_lock(&(pool->lock)) != thrd_success) {
        return threadpool_lock_failure;
    }

    next = (pool->tail + 1) % pool->queue_size;

    do {
        /* Are we shutting down ? */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Are we full ? */
        if((pool->count+1) >= pool->queue_size) {
            cnd_wait(&(pool->queue_notify), &(pool->lock));
        }

        /* Add task to queue */
        pool->queue[pool->tail].function = function;
        memcpy(&pool->queue[pool->tail].argument, argument, sizeof(Point));
        pool->tail = next;
        pool->count += 1;

        /* pthread_cond_broadcast */
        if(cnd_signal(&(pool->notify)) != thrd_success) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    if(mtx_unlock(&pool->lock) != thrd_success) {
        err = threadpool_lock_failure;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return threadpool_invalid;
    }

    if(mtx_lock(&(pool->lock)) != thrd_success) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        pool->shutdown = (flags & threadpool_graceful) ?
            graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        if((cnd_broadcast(&(pool->notify)) != thrd_success) ||
           (mtx_unlock(&(pool->lock)) != thrd_success)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        for(i = 0; i < pool->thread_count; i++) {
            if(thrd_join(pool->threads[i], NULL) != thrd_success) {
                err = threadpool_thread_failure;
            }
        }
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL || pool->started > 0) {
        return -1;
    }

    /* Did we manage to allocate ? */
    if(pool->threads) {
        free(pool->threads);
        pool->threads = NULL;

        if(pool->queue) {
            free(pool->queue);
            pool->queue = NULL;
        }

        /* Because we allocate pool->threads after initializing the
           mutex and condition variable, we're sure they're
           initialized. Let's lock the mutex just in case. */
        mtx_lock(&(pool->lock));
        mtx_destroy(&(pool->lock));
        cnd_destroy(&(pool->notify));
        cnd_destroy(&(pool->queue_notify));
        cnd_destroy(&(pool->all_tasks_done));
    }
    free(pool);
    return 0;
}


static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;
    
    if (pool == NULL) {
        thrd_exit(0);
        return NULL;
    }


    for(;;) {
         /* Lock must be taken to wait on conditional variable */
        mtx_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from cnd_wait(), we own the lock. */
        while((pool->count == 0) && (!pool->shutdown)) {
            cnd_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == immediate_shutdown) ||
           ((pool->shutdown == graceful_shutdown) &&
            (pool->count == 0))) {
            break;
        }

        /* Grab our task */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument; // Copia directa del struct Point

        //DebugPrint(" Arranca Hilo %d, hay tareas %d\n",(pool->head + 1) % pool->queue_size,pool->count-1);
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;
        
        cnd_signal(&(pool->queue_notify));
        
        /* Unlock */
        mtx_unlock(&(pool->lock));

        /* Get to work */
        if (task.function != NULL) {
            (*(task.function))(&task.argument);
        }
        
        /* Lock the mutex before accessing the task counter and condition variable */
        mtx_lock(&(pool->lock));

        //DebugPrint(" i:%d , j:%d , tam:%d\n",((punto*)(task.argument))->i,((punto*)(task.argument))->j,((punto*)(task.argument))->tam);    
        /* If all tasks are done, signal the condition variable */
        if(pool->count == 0) {
            cnd_signal(&(pool->all_tasks_done));
        }        
        /* Unlock the mutex */
        mtx_unlock(&(pool->lock));
    }

    // No cleanup needed for Point struct
    
    pool->started--;

    if(pool->count == 0) {
        cnd_signal(&(pool->all_tasks_done));
    }

    mtx_unlock(&(pool->lock));
    thrd_exit(0);
    return NULL;
}

/* En el hilo principal, espera a que todas las tareas se hayan completado */
void threadpool_wait_all(threadpool_t *pool) {
    mtx_lock(&(pool->lock));
    if (pool->count>0) {
        cnd_wait(&(pool->all_tasks_done), &(pool->lock));
    }
    mtx_unlock(&(pool->lock));
}