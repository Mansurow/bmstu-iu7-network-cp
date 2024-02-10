#include "thread_pool.h"

thread_task_t *thread_task_create(thread_func_t func, void *args);
void thread_task_destroy(thread_task_t *task);
thread_task_t *threadpool_get_task(threadpool_t *pool);

void *thread_execute(void *args);


threadpool_t *threadpool_create(uint32_t numthreads)
{
    if (numthreads <= 0)
        return NULL;

    threadpool_t *pool = (threadpool_t *) calloc(1, sizeof(threadpool_t));

    if (pool != NULL)
    {
        pool->threads = (pthread_t *) calloc(numthreads, sizeof(pthread_t));
        if (pool->threads == NULL)
        {
            free(pool);
            pool = NULL;
        }
        else if (pthread_cond_init(&(pool->work_cond), NULL) != 0)
        {
            free(pool->threads);
            free(pool);
            pool = NULL;
        }
        // else if (pthread_cond_init(&(pool->task_queue_empty), NULL) != 0)
        // {
        //     pthread_cond_destroy(&(pool->work_cond));
        //     free(pool->threads);
        //     free(pool);
        //     pool = NULL;
        // }
        else if (pthread_mutex_init(&(pool->work_mutex), NULL) != 0)
        {
            //pthread_cond_destroy(&(pool->task_queue_empty));
            pthread_cond_destroy(&(pool->work_cond));
            free(pool->threads);
            free(pool);
            pool = NULL;
        }
        else
        {
            pool->numthreads = numthreads;
            pool->destroying = 0;
            pool->numtasks = 0;
            pool->head = NULL;
            pool->tail = NULL;

            pool->stats_info.total_tasks = 0;
            pool->stats_info.total_handled_task = 0;

            for (uint32_t i = 0; i < numthreads; i++)
            {
                pthread_create(pool->threads + i, NULL, thread_execute, pool);
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
        // while (pool->head != NULL) 
        // {
        //     pthread_cond_wait(&(pool->task_queue_empty), &(pool->mutex));
        // }

        pthread_mutex_lock(&(pool->work_mutex));
        pool->destroying = 1;
        pthread_mutex_unlock(&(pool->work_mutex));

        pthread_cond_broadcast(&(pool->work_cond));
        for (uint32_t i = 0; i < pool->numthreads; i++)
        {
            pthread_join(pool->threads[i], NULL);
        }

        res |= pthread_mutex_destroy(&(pool->work_mutex));
        res |= pthread_cond_destroy(&(pool->work_cond));

        printf("Всего пришло задач в очерель: %ld\n", pool->stats_info.total_tasks);
        printf("Всего обраблотано задач: %d\n", pool->stats_info.total_handled_task);

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
        threadpool_t *pool = args;
        
        while (!pool->destroying)
        {
            pthread_mutex_lock(&(pool->work_mutex));
            if(!pool->destroying && !(pool->head))
            {
                pthread_cond_wait(&(pool->work_cond), &(pool->work_mutex));
            }

            thread_task_t *task = threadpool_get_task(pool);
            
            while (task != NULL)
            {
                pthread_mutex_unlock(&(pool->work_mutex));
                task->func(task->args);
                thread_task_destroy(task);
                task = NULL;
                pthread_mutex_lock(&(pool->work_mutex));
                task = threadpool_get_task(pool);
            }
            pthread_mutex_unlock(&(pool->work_mutex));
        }
    }

    return NULL;
}

thread_task_t *threadpool_get_task(threadpool_t *pool)
{
    thread_task_t *task = NULL;
    if (pool && pool->head)
    {
        task = pool->head;
        pool->head = pool->head->next;
        if (!pool->head)
        {
            pool->tail = NULL;
        }

        pool->numtasks--;
        pool->stats_info.total_handled_task++;
    }
    return task;
}

int threadpool_task_add(threadpool_t *pool, thread_func_t func, void *args)
{
    thread_task_t *task = NULL;
    if (pool != NULL && (task = thread_task_create(func, args)))
    {
        // Критический участок
        pthread_mutex_lock(&(pool->work_mutex));
        // ДВУСВЯЗАННЫЙ СПИСОК!
        if (pool->head != NULL)
        {
            pool->tail->next = task;
            // task->prev = pool->tail;
        } 
        else
        {
            pool->head = task;
        }

        pool->tail = task;
        pool->numtasks++;

        pool->stats_info.total_tasks++;

        pthread_mutex_unlock(&(pool->work_mutex));

        // разблокировка хотя бы одного сигнала
        pthread_cond_signal(&(pool->work_cond));
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
        // task->prev = NULL;
    }
    return task;
}

void thread_task_destroy(thread_task_t *task)
{
    free(task);
}