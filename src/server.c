#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


// подумать на входными параметрами (количество потоков, соединений...)
int main() {

    // создаем сокет
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        // сокет не создался
        exit(1);
    }

    struct sockaddr_in listen_sockaddr;
    listen_sockaddr.sin_family = AF_INET;
    listen_sockaddr.sin_addr.s_addr = INADDR_ANY;
    listen_sockaddr.sin_port = htons(8080);

    // связываем физический адрес с сокетом
    if(bind(sock_fd, (struct sockaddr *) &listen_sockaddr, sizeof(listen_sockaddr)) == -1)
    {
        // сокет не связаялся
        exit(1);
    }

    // прослушиваем сокет для установки соединения
    if (listen(sock_fd, 1) == -1) 
    {
        // соединение не установлено с сокетои
        exit(1);
    }

    int nfds = sock_fd + 1;
    fd_set client_fdset;
    FD_ZERO(&client_fdset);
    FD_SET(sock_fd, &client_fdset);

    struct timespec timeout;
    timeout.tv_sec = 5; // Ожидание в течение 5 секунд
    timeout.tv_nsec = 0;

    while(1) {

        int res = pselect(nfds, &client_fdset, NULL, NULL, &timeout, NULL);
        if (res == -1)
        {
            perror("pselect");
            // Ошибка ожидания изменения в сокете
            exit(1);
        }
        else if (res == 0)
        {
             printf("Timeout occurred\n");
            // Ожидание вышло
        }
        else 
        {
            printf("Socket ready for accept\n");
            // Прием соединения
            if (FD_ISSET(sock_fd, &client_fdset))
            {
                struct sockaddr_in client_addr;
	            socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(sock_fd, (struct sockaddr *) &client_addr, &client_len);
            }
        }  
    }

    close(sock_fd);

    return 0;
}