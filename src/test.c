#include "thread_pool.h"
#include "logger.h"

void *thread_pool_execute(void *arg);

void *thread_fun(void *arg) {
    printf("%d\n", *(int *) arg);
    return NULL;
}

int main()
{
    logger_create("./test.log");

    threadpool_t *pool = threadpool_create(8);
    printf("threadpool: %p\n", (void *) pool);

    int arg[21] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

    for (int i = 0; i < 21; i ++ ) {
        threadpool_task_add(pool, thread_fun, (void *) (arg + i));
    }
    
    logger_log("Hello World!");

    threadpool_destroy(pool);
    logger_destroy();

    return 0;
}