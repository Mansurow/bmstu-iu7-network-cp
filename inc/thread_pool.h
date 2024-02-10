# ifndef __THREADPOOL_H
# define __THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#define DEFAULT_QUEUE_LEN 5

typedef void (*thread_func_t)(void *);

typedef struct thread_task_t
{
    thread_func_t func;
    void *args;
    struct thread_task_t *next;
    struct thread_task_t *prev;
} thread_task_t;

typedef struct queue_stats_t
{
    unsigned int total_tasks;
    unsigned int total_handled_task; 
} queue_stats_t;

typedef struct
{
    thread_task_t *head;
    thread_task_t *tail;

    pthread_cond_t task_queue_empty;
    // pthread_cond_t task_queue_empty;
    // pthread_cond_t task_queue_not_empty;
    // pthread_cond_t task_queue_not_full;
    pthread_mutex_t mutex;
    pthread_t *threads;   

    uint32_t numtasks;    // количество задач на выполнение == длине списка задач
    uint32_t destroying;  // флаг процесса уничтожения потоков
    uint32_t numthreads;  // количество потоков

    queue_stats_t stats_info;
} threadpool_t;

threadpool_t *threadpool_create(uint32_t numthreads);
int threadpool_destroy(threadpool_t *threadpool);
int threadpool_task_add(threadpool_t *threadpool, thread_func_t func, void *args);

# endif