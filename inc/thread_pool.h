# ifndef __THREADPOOL_H
# define __THREADPOOL_H

# include <pthread.h>

typedef void (*thread_func_t)(void *);

typedef struct thread_task_t
{
    thread_func_t func;
    void *args;
    struct thread_task_t *next;
    struct thread_task_t *prev;
} thread_task_t;

typedef struct
{
    thread_task_t *head;
    thread_task_t *tail;

    pthread_cond_t work_cond;
    pthread_mutex_t work_mutex;
    pthread_t *threads;   

    uint32_t numtasks;    // количество задач на выполнение == длине списка задач
    uint32_t destroying;  // флаг процесса уничтожения потоков
    uint32_t numthreads;  // количество потоков
} threadpool_t;

threadpool_t *threadpool_create(uint32_t number_of_threads);
int threadpool_destroy(threadpool_t *threadpool);
int threadpool_work_add(threadpool_t *threadpool, thread_func_t func, void *args);

# endif