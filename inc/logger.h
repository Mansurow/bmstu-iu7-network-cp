# ifndef __LOGGER_H
# define __LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_LOG_FILE "server.log"
#define DEFAULT_LOG_DIR  "logs"
#define DEFAULT_LOG_PATH "./" DEFAULT_LOG_DIR "/" DEFAULT_LOG_FILE

#define LOG_BUF_LEN 1024

typedef enum log_type_t
{
    INFO     = 0,
    DEBUG    = 1,
    WARNING  = 2,
    ERROR    = 3,
    CRITICAL = 4,
    FATAL    = 5,
    TRACE    = 6
} log_type_t;

typedef struct logger_node_t
{
    char *message;
    log_type_t type;
    struct logger_node_t *next;
} logger_node_t;

typedef struct
{
    int active;      // логи будут записываться?
    int destroying;  // флаг уничтожения потока логирования

    char *filename;

    logger_node_t *head;
    logger_node_t *tail;

    pthread_t thread;
    pthread_cond_t work_cond;
    pthread_mutex_t mutex;
} logger_t;

extern logger_t *logger;

void logger_create(const char *filename);
int logger_destroy();
void logger_log(log_type_t type, const char *msg);

#endif