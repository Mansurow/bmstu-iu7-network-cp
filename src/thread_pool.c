#include "thread_pool.h"

thread_task_t *thread_task_create(thread_func_t func, void *args);
void thread_task_destroy(thread_task_t *task);

void *thread_execute(void *args);

threadpool_t *threadpool_create(uint32_t numthreads)
{
    if (number_of_threads <= 0)
        return NULL;

    threadpool_t *pool = (threadpool_t *) calloc(1, sizeof(threadpool_t));

    if (pool != NULL)
    {
        pool->threads = (pthread_t *) calloc(numthreads, sizeof(pthread_t));
        if (poo->threads == NULL)
        {
            free(pool);
            pool = NULL:
        }
        else if (pthread_cond_init(&(pool->work_cond), NULL) != 0)
        {
            free(pool->threads);
            free(pool);
            pool = NULL:
        }
        else if (pthread_mutex_init(&(pool->work_mutex), NULL) != 0)
        {
            pthread_cond_destroy(&(pool->work_cond));
            free(pool->threads);
            free(pool);
            pool = NULL:
        }
        else
        {
            pool->numthreads = numthreads;
            pool->destroying = 0;
            pool->numtasks = 0;
            pool->head = NULL;
            pool->tail = NULL;


            for (uint32_t i = 0; i < numthreads; i++)
            {
                thread_create(pool->threads + i, NULL, thread_execute, pool);
            }
        }
    }   

    return pool; 
}

int threadpool_destroy(threadpool_t *pool)
{
    int res = 0;
    if (pool != NULL)
    {
        pthread_mutex_lock(&(pool->work_mutex));
        pool->destroying = 1;
        pthread_mutex_unlock(&(pool->work_mutex));

        pthread_cond_broadcast(&(pool->work_cond));
        for (uint32_t i = 0; i < pool->numthreads; i++)
        {
            pthread_join(pool->threads + i, NULL);
        }

        res |= pthread_mutex_destroy(&(pool->work_mutex));
        res |= pthread_cond_destroy(&(pool->work_cond));
        if (!res)
        {
            free(pool);
            pool = NULL;
        }
    }

    return res;
}

void *thread_execute(void *args)
{
    if (args)
    {

    }

    return NULL;
}

int threadpool_task_add(threadpool_t *pool, thread_func_t func, void *args)
{
    thread_task_t *task = NULL;
    if (pool != NULL)
    {
        task = thread_task_create(func. args);
        if (task != NULL)
        {
            
        }
    }

    return task != NULL;
}

thread_task_t *thread_task_create(thread_func_t func, void *args)
{
    thread_task_t *task = calloc(1, sizeof(thread_task_t));
    if (task != NULL)
    {
        task->func = func;
        task->args = args;
        task->next = NULL;
    }
    return work;
}

void thread_task_destroy(thread_task_t *task)
{
    free(task);
}