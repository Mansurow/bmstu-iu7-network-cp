#include "server.h"
#include "thread_pool.h"
#include "http.h"
#include "logger.h"

static int stop_flag = 0; 

void http_handler(int socket)
{
    http_header_t header;

    char request_data[MAX_GET_REQUEST + 1] = {0};
    char log_buff[LOG_BUF_LEN * 2 + 1] = { 0 };

    long bytes = read(socket, request_data, MAX_GET_REQUEST);
    
    sprintf(log_buff, "На сокет %d пришёл запрос", socket);
    logger_log(DEBUG, log_buff);   

    header = parse_header(request_data);

    if (header.response == OK)
    {
        char *responde_header = get_content_type_header(header.file_type);

        // printf("%s\n", responde_header);

        write(socket, responde_header, strlen(responde_header));

        FILE *file = fopen(header.path, "r");
        if (file)
        {

            char data[BUFFER_LEN] = {0};
            size_t number_of_data = 0;

            while ((number_of_data = fread(data, sizeof(char), BUFFER_LEN, file)))
                write(socket, data, number_of_data);

            fclose(file);
        }
        else 
        {
            write(socket, HTTP_500, HTTP_500_LEN);
        }

        free(responde_header);
    }
    else if (header.response == FORBIDDEN)
    {
        write(socket, HTTP_403, HTTP_403_LEN);
    }
    else if (header.response == NOT_FOUND)
    {
        write(socket, HTTP_404, HTTP_404_LEN);
    }
    else if (header.response == METHOD_NOT_ALLOWED)
    {
        write(socket, HTTP_405, HTTP_405_LEN);
    }

    close(socket);
    if (header.path != NULL)
        free(header.path);

    sprintf(log_buff, "На сокет %d закрыто соединение", socket);
    logger_log(DEBUG, log_buff);   
}

void sigint_handler() 
{
    printf("\nБыло нажато Ctrl+C, сервер завершается...\n");
    stop_flag = 1;
}

int main(int argc, char *argv[]) 
{
    int port = DEFAULT_PORT;
    char *dir = DEFAULT_ROOT_DIR;

    char log_buff[LOG_BUF_LEN * 2 + 1] = { 0 };

    logger_create(DEFAULT_LOG_FILE);

    if (chdir(DEFAULT_ROOT_DIR) == 0) 
    {
        sprintf(log_buff, "Успешно назначена новая корневая директория: %s", DEFAULT_ROOT_DIR);
        logger_log(INFO, log_buff);
    } 
    else 
    {
        sprintf(log_buff, "Корневая директория не назначена. Ошибка в chdir, errno=%s", strerror(errno));
        logger_log(ERROR, log_buff);
        logger_destroy();
        exit(1);
    }

    ssignal(SIGPIPE, SIG_IGN);
    // signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, sigint_handler);

    // struct sigaction sa;
    // sigset_t emptyset, blockset;
    // sigemptyset(&blockset);         /* Block SIGINT */
    // sigaddset(&blockset, SIGINT);
    // sigprocmask(SIG_BLOCK, &blockset, NULL);
    // sa.sa_handler = sigint_handler;        /* Establish signal handler */
    // sa.sa_flags = 0;
	// sigemptyset(&sa.sa_mask);
    // sigaction(SIGINT, &sa, NULL);

    // sigemptyset(&emptyset);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        sprintf(log_buff, "Не удалось создать сокет. Ошибка в socket, errno=%s", strerror(errno));
        logger_log(ERROR, log_buff);
        logger_destroy();   
        exit(1);
    }

    struct sockaddr_in listen_sockaddr;
    listen_sockaddr.sin_family = AF_INET;
    listen_sockaddr.sin_addr.s_addr = INADDR_ANY;
    listen_sockaddr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr *) &listen_sockaddr, sizeof(listen_sockaddr)) == -1)
    {
        sprintf(log_buff, "Не удалось назначить адрес сокету. Ошибка в bind, errno=%s", strerror(errno));
        logger_log(ERROR, log_buff);
        logger_destroy();     
        exit(1);
    }

    if (listen(listenfd, LISTEN_BACKLOG) == -1) 
    {
        sprintf(log_buff, "Не удалось начать слушать сокет для установления соединения. Ошибка в listen, errno=%s", strerror(errno));
        logger_log(ERROR, log_buff);
        logger_destroy();
        exit(1);
    }

    logger_log(DEBUG, "Сокет для прослушивания создан и ему назначен адрес");

    long number_processors = sysconf(_SC_NPROCESSORS_ONLN) - 1; 
    threadpool_t *threadpool = threadpool_create(number_processors);
    if (threadpool == NULL)
    {
        logger_log(ERROR, "Ошибка в threadpool_create");
        close(listenfd);
        logger_destroy();
        exit(1);
    }
    
    sprintf(log_buff, "Создано %u потоков thread pool", number_processors);
    logger_log(DEBUG, log_buff);

    printf("Нажмите Ctrl+C, чтобы закрыть сервер");

    fd_set default_fds, changing_fds;
    FD_ZERO(&default_fds);
    FD_ZERO(&changing_fds);
    FD_SET(listenfd, &default_fds);
    int nfds = listenfd + 1;

    // настройка времени ожидания
    // struct timespec timeout;
    // timeout.tv_sec = 5; // Ожидание в течение 5 секунд
    // timeout.tv_nsec = 0;


    // настройка массок сигналов для блокировки
    // sigset_t sigmask;
    // sigemptyset(&sigmask);

    while(!stop_flag) 
    {
        changing_fds = default_fds;

        int res = pselect(nfds, &changing_fds, NULL, NULL, NULL, NULL);
        if (res == -1)
        {
            sprintf(log_buff, "Не удалось отследить изменение сокета. Ошибка в pselect, errno=%s", strerror(errno));
            logger_log(ERROR, log_buff);
            continue;
        }

        for (int nfd = 0; nfd < nfds; nfd++)
        {
            if (FD_ISSET(nfd, &changing_fds))
            {
                if (nfd == listenfd)
                {
                    // struct sockaddr_in client_addr;
                    // socklen_t client_len = sizeof(client_addr);
                    int clientfd = -1;
                    if ((clientfd = accept(listenfd, NULL, NULL)) == -1)
                    {
                        sprintf(log_buff, "Соединение с клиентом не установлено. Ошибка в accept, errno=%s", strerror(errno));
                        logger_log(ERROR, log_buff);
                        // close(listenfd);
                    }
                    else 
                    {
                        sprintf(log_buff, "Соединились с клиентом по сокету %d", clientfd);
                        logger_log(DEBUG, log_buff);

                        FD_SET(clientfd, &default_fds);
                        if (clientfd >= nfds)
                            nfds = clientfd + 1;

                        // http_handler(clientfd);
                    }            
                } 
                else
                {
                    FD_CLR(nfd, &default_fds);

                    sprintf(log_buff, "Загрузка работы с сокетом %d в очередь", nfd);
                    logger_log(DEBUG, log_buff);

                    threadpool_task_add(threadpool, http_handler, (void *) nfd);   
                }
            }
        }  
    }

    threadpool_destroy(threadpool);
    close(listenfd);

    logger_log(DEBUG, "Сокет для прослушивания закрыт");
    logger_log(DEBUG, "Потоки thread pool уничтожены");
    logger_log(INFO, "Сервер был завершен");

    logger_destroy();

    return 0;
}