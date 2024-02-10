# ifndef __LOGGER_H
# define __LOGGER_H

#define DEFAULT_LOG_FILE "server.log"
#define DEFAULT_LOG_DIR  "logs"
#define DEFAULT_PATH     "./" DEFAULT_LOG_DIR "/" DEFAULT_LOG_FILE

typedef struct logger_node_t
{
    char *message;
    struct logger_node_t *next;
} logger_node_t;

typedef struct
{
    int active;      // логи будут записываться?
    int destroying;  // флаг уничтожения потока логирования

    char *filename;

    logger_node_t *head;
    logger_node_t *tale;

    pthread_t worker;
    pthread_cond_t work_cond;
    pthread_mutex_t work_mutex;
} logger_t;

logger_t *logger;

void logger_create(const char *filename);
int logger_destroy();

#endif