#include "logger.h"

logger_t *logger = NULL;

logger_node_t *node_create(const char *message);
logger_node_t *logger_get_node(void);
void node_destroy(logger_node_t *node);
void fprintf_log(FILE *file, logger_node_t *node);

void *logger_execute(void *args);

void logger_create(const char *filename)
{
    if (logger != NULL)
    {
        return;
    }

    logger = (logger_t *) calloc(1, sizeof(logger_t));
    size_t len = strlen(filename);
    char *f = calloc(len + 1, sizeof(char));
    
    if (logger == NULL || f == NULL)
    {
        free(f);
        free(logger);
    }
    else if (pthread_cond_init(&(logger->work_cond), NULL) != 0)
    {
        free(f);
        free(logger);
    }
    else if (pthread_mutex_init(&(logger->mutex), NULL) != 0)
    {
        pthread_cond_destroy(&(logger->work_cond));
        free(f);
        free(logger);
    }
    else
    {
        logger->active = 1;
        logger->destroying = 0;
        logger->head = NULL;
        logger->tail = NULL;
        logger->filename = f;
        memcpy(logger->filename, filename, len);

        pthread_create(&(logger->thread), NULL, logger_execute, NULL);
    }
}

void *logger_execute(void *args)
{
    while (!logger->destroying)
    {
        pthread_mutex_lock(&(logger->mutex));
        
        if (!logger->destroying && !logger->head)
        {
            pthread_cond_wait(&(logger->work_cond), &(logger->mutex));
        }

        logger_node_t *node = logger_get_node();
        pthread_mutex_unlock(&(logger->mutex));

        if (node != NULL)
        {   
            FILE *file = fopen(logger->filename, "a");
            
            while(node)
            {   
                fprintf_log(file, node);
                node_destroy(node);

                pthread_mutex_lock(&(logger->mutex));
                node = logger_get_node();
                pthread_mutex_unlock(&(logger->mutex));
            }

            fclose(file);
        }
    }

    return NULL;
}

int logger_destroy()
{
    int result = 0;

    if (logger != NULL)
    {
        pthread_mutex_lock(&(logger->mutex));
        logger->destroying = 1;
        pthread_mutex_unlock(&(logger->mutex));

        pthread_cond_broadcast(&(logger->work_cond));

        pthread_join(logger->thread, NULL);

        result |= pthread_cond_destroy(&(logger->work_cond));
        result |= pthread_mutex_destroy(&(logger->mutex));
        if (!result)
        {
            free(logger);
        }
    }

    return result;
}

logger_node_t *node_create(const char *message)
{
    logger_node_t *node = (logger_node_t *) calloc(1, sizeof(logger_node_t));
    
    if (node != NULL)
    {
        size_t len = strlen(message);
        char *tmp = (char *) calloc(len + 1, sizeof(char));
        if (tmp == NULL)
        {
            free(node);
            node = NULL;
        }
        else
        {
            node->next = NULL;
            node->message = tmp;
            memcpy(node->message, message, len);
        }
    }

    return node;
}

logger_node_t *logger_get_node()
{
    logger_node_t *node = NULL;

    if (logger != NULL && logger->head != NULL)
    {
        node = logger->head;
        logger->head = logger->head->next;
        
        if (logger->head == NULL)
        {
            logger->tail = NULL;
        }
    }

    return node;
}

void node_destroy(logger_node_t *node)
{
    if (node != NULL)
    {
        free(node->message);
        free(node);
    }
}

void fprintf_log(FILE *file, logger_node_t *node)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(file, "<%d-%02d-%02d %02d:%02d:%02d> ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(file, "%s\n", node->message);
}

void logger_log(const char *msg)
{
    printf("Message: %s\n", msg);

    if (logger != NULL)
    {
        logger_node_t *node = node_create(msg);

        if (node != NULL)
        {
            printf("Message: %s\n", node->message);

            pthread_mutex_lock(&(logger->mutex));

            if (logger->head != NULL)
            {
                logger->tail->next = node;
            }
            else
            {
                logger->head = node;
            }

            logger->tail = node;

            pthread_mutex_unlock(&(logger->mutex));

            pthread_cond_signal(&(logger->work_cond));
        }
    }
}