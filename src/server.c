#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SOCKET "server.socket"

#define BUFFER_LEN      1024
#define MAX_GET_REQUEST 2048
#define MAX_CLIENTS     32

# define MAX_HTTP_METHOD_LEN  7
# define MAX_HTTP_VERSION_LEN 5

#define HTTP_200 "HTTP/1.1 200 OK\r\n"\
	"Server: Static Server\r\n"\
	"Connection: Closed\r\n"\
	"Content-Type: "

#define HTTP_403 "HTTP/1.1 403 Forbidden\r\n"\
	"Server: Static Server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Connection: Closed\r\n\r\n"

#define HTTP_404 "HTTP/1.1 404 Not Found\r\n"\
	"Server: Static Server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Connection: Closed\r\n\r\n"

#define HTTP_405 "HTTP/1.1 405 Method Not Allowed\r\n"\
	"Server: Static Server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Connection: Closed\r\n\r\n"

#define HTTP_500 "HTTP/1.1 500 Internal Server Error\r\n"\
	"Server: Static Server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Connection: Closed\r\n\r\n"    

#define HTTP_200_LEN strlen(HTTP_200)
#define HTTP_403_LEN strlen(HTTP_403)
#define HTTP_404_LEN strlen(HTTP_404)
#define HTTP_405_LEN strlen(HTTP_405)
#define HTTP_500_LEN strlen(HTTP_500)

typedef enum file_type
{
    NONE = 0,
	HTML = 1,
	CSS = 2,
	JS = 3,
	JPG= 4,
	JPEG = 5,
	SWF = 6,
	GIF = 7,
	PNG = 8,
	TXT = 9,
	SVG = 10
} file_type_t;

typedef enum http_request {
    UNKNOWN = 0,
    GET = 1,
    HEAD = 2
} http_request_t;

typedef enum http_reponse {
    DEFAULT = 0,
    CONTINUE = 100,
    PROCESSING = 102,
    OK = 200,
    NO_CONTENT = 204,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
    CONFLICT = 409,
    METHOD_NOT_ALLOWED = 405,
    NOT_IMPLEMENTED = 501,
} http_reponse_t;

typedef enum http_version {
    http1 = 1,
    http1_1 = 11,
    http2 = 2,
    http3 = 3
} http_version_t;

typedef struct http_header {
    char *path;
    http_version_t version;
    http_request_t request;
    file_type_t file_type;
    http_reponse_t response;
} http_header_t;

static int stop_flag = 0; 

void init_header(http_header_t *header)
{   
    header->file_type = NONE;
    header->request = UNKNOWN;
    header->response = DEFAULT;
    header->version = http1;
    header->path = NULL;   
}

bool is_avaliable_path(const char *path)
{
    if (strstr(path, "..") || strstr(path, "~") || strstr(path, "/."))
    {
        return false;
    }

    return true;
}

http_request_t convert_to_request_type(const char *method)
{
    http_request_t req = UNKNOWN;

    if (!strcmp(method, "GET"))
    {
        req = GET;
        printf("GET\n");
    }
    else if (!strcmp(method, "HEAD"))
    {
        req = HEAD;
        printf("HEAD\n");
    }

    return req;
}

http_version_t convert_to_version_type(const char *version)
{
    http_version_t req = http1;

    if (!strcmp(version, "1"))
    {
        req = http1;
    }
    else if (!strcmp(version, "1.1"))
    {
        req = http1_1;
    }
    else if (!strcmp(version, "2"))
    {
        req = http2;
    }
    else if (!strcmp(version, "3"))
    {
        req = http3;
    }

    return req;
}

file_type_t convert_to_file_type(char *path)
{
    int i = 1;
    char c;
    file_type_t type = NONE;
    while ((c = path[i++]) && c != '.');
    if (c)
    {
        char extension[BUFFER_LEN + 1] = {0};
        int len = strlen(path);
        memcpy(extension, path + i, len - i);
        if (!strcmp(extension, "html"))
            type = HTML;
        else if (!strcmp(extension, "css"))
            type = CSS;
        else if (!strcmp(extension, "js"))
            type = JS;
        else if (!strcmp(extension, "png"))
            type = PNG;
        else if (!strcmp(extension, "jpg"))
            type = JPG;
        else if (!strcmp(extension, "jpeg"))
            type = JPG;
        else if (!strcmp(extension, "swf"))
            type = SWF;
        else if (!strcmp(extension, "gif"))
            type = GIF;
        else if (!strcmp(extension, "txt"))
            type = TXT;
        else
            type = NONE;
    }
    return type;
}

http_header_t parse_header(const char *buff)
{
    http_header_t header;

    char method_buff[MAX_HTTP_METHOD_LEN + 1] = { 0 };
    char vesrion_buff[MAX_HTTP_VERSION_LEN + 1] = { 0 };

    size_t path_len = 0, count_space = 0, i = 0;
    while (buff[i] != '\0' && buff[i] != '\n') 
    {
        if (count_space == 1)
        {
            path_len++;
        }

        if (buff[i] == ' ')
        {
            count_space++;
        }

        i++;
    }

    char *header_path = (char *) calloc(sizeof(char), path_len + 1);
    
    if (sscanf(buff, "%s %s HTTP/%s", method_buff, header_path, vesrion_buff) == 3)
    {
        header.request = convert_to_request_type(method_buff);
        header.version = convert_to_version_type(vesrion_buff);
        
        if (!is_avaliable_path(header_path)) 
        {
            header.response = FORBIDDEN;
            header.path = header_path;
        }
        else 
        {
            char *path = (char *) calloc(sizeof(char), path_len + 12);
            if (!strcmp(header_path, "/"))
            {
                sprintf(path, "./index.html");
                header.path = path;
            } 
            else
            {
                sprintf(path, ".%s", header_path);
                header.path = path;
            }

            if (access(header.path, R_OK))    
            {
                header.response = NOT_FOUND;
            } 
            else
            {
                header.response = OK;
            }
        }

        header.file_type = convert_to_file_type(header.path);
    }  

    
    printf("len: %d\n", path_len);
    printf("spaces: %d\n", count_space);

    printf("FILE TYPE: %d\n", header.file_type);
    printf("METHOD: %d\n", header.request);
    printf("VESION: %d\n", header.version);
    printf("PATH: %s\n", header.path);
    printf("RESPONSE: %d\n", header.response);

    free(header_path);

    return header;
}

char *get_content_type_header(file_type_t ft)
{
    char *response_header = (char *) calloc(1, HTTP_200_LEN + 53);
    switch(ft)
	{
		case HTML:
			sprintf(response_header, "%stext/html; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case CSS:
			sprintf(response_header, "%stext/css; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case JS:
			sprintf(response_header, "%stext/javascript; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case SWF:
			sprintf(response_header, "%sapplication/x-shockwave-flash; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case GIF:
			sprintf(response_header, "%simage/gif; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case TXT:
			sprintf(response_header, "%stext/plain; charset=utf-8\r\n\r\n", HTTP_200);
			break;
		case JPG:
			sprintf(response_header, "%simage/jpeg\r\n\r\n", HTTP_200);
			break;
		case PNG:
			sprintf(response_header, "%simage/png\r\n\r\n", HTTP_200);
			break;
		case SVG:
			sprintf(response_header, "%simage/svg+xml\r\n\r\n", HTTP_200);
			break;
		default:
			sprintf(response_header, "%sapplication/octet-stream\r\n\r\n", HTTP_200);
			break;
	}

    return response_header;
}

void http_handler(int socket)
{
    http_header_t header;

    init_header(&header);

    char header_buf[BUFFER_LEN] = { 0 };
 
    char request_data[MAX_GET_REQUEST + 1] = {0};
    long bytes = read(socket, request_data, MAX_GET_REQUEST);
    // char log_buffer[MAX_GET_REQUEST * 2 + 1] = {0};
    printf("На сокет %d пришёл запрос\n", socket);

    header = parse_header(request_data);

    printf("1111 На сокет %d пришёл запрос\n", socket);

    if (header.request == UNKNOWN)
    {
        header.response = METHOD_NOT_ALLOWED;
    }

    if (header.response == OK)
    {
        char *responde_header = get_content_type_header(header.file_type);

        printf("%s\n", responde_header);

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

    printf("request type: %d\n", header.request);

    close(socket);
    // free(header.path);
}

void sigint_handler() 
{
    printf("Было нажато Ctrl+C, сервер завершается...\n");
    stop_flag = 1;
}

// подумать на входными параметрами (количество потоков, соединений...)
int main() {

    if (chdir("./static") == 0) {
        printf("Успешно назначена новая корневая директория: %s\n", "/static");
    } else {
        fprintf(stderr, "Ошибка при назначении новой корневой директории\n");
        exit(-1);
    }

    // ssignal(SIGPIPE, SIG_IGN);
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

    /* Initialize nfds and readfds, and perhaps do other work here */
    /* Unblock signal, then wait for signal or ready file descriptor */

    // sigemptyset(&emptyset);

    printf("Нажмите Ctrl+C, чтобы закрыть сервер\n");

    // создаем сокет
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        printf("Не удалось создать сокет\n");
        // сокет не создался
        exit(1);
    }

    struct sockaddr_in listen_sockaddr;
    listen_sockaddr.sin_family = AF_INET;
    listen_sockaddr.sin_addr.s_addr = INADDR_ANY;
    listen_sockaddr.sin_port = htons(8089);
    // strcpy(listen_sockaddr.sin_zero, SOCKET);

    // связываем физический адрес с сокетом
    if(bind(listenfd, (struct sockaddr *) &listen_sockaddr, sizeof(listen_sockaddr)) == -1)
    {
        printf("Не удалось назначить адрес сокету\n");
        // сокет не связаялся
        exit(1);
    }

    // прослушиваем сокет для установки соединения
    if (listen(listenfd, MAX_CLIENTS) == -1) 
    {
        printf("Не удалось начать прослушивать сокет\n");
        // соединение не установлено с сокетои
        exit(1);
    }

    // создание thread_pool

    fd_set default_fds, changing_fds;
    FD_ZERO(&default_fds);
    FD_ZERO(&changing_fds);
    FD_SET(listenfd, &changing_fds);
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
        int res = pselect(nfds, &changing_fds, NULL, NULL, NULL, NULL);
        if (res == -1)
        {
            perror("pselect");
            // Ошибка ожидания изменения в сокете
            exit(1);
        }

        for (int nfd = 0; nfd < nfds; nfd++)
        {
            // Прием соединения
            if (FD_ISSET(nfd, &changing_fds))
            {
                if (nfd == listenfd)
                {
                    // struct sockaddr_in client_addr;
                    // socklen_t client_len = sizeof(client_addr);
                    int clientfd = -1;
                    if ((clientfd = accept(listenfd, NULL, NULL)) == -1)
                    {
                        close(listenfd);
                        printf("Error Accept\n");
                        // exit(1);
                    }
                    else 
                    {
                        printf("Соединились с клиентом по сокету %d\n", clientfd);

                        FD_SET(clientfd, &default_fds);
                        if (clientfd >= nfds)
                            nfds = clientfd + 1;

                        http_handler(clientfd);
                    }            
                } 
                else
                {
                    FD_CLR(nfd, &default_fds);
                }
            }
        }  
    }

    close(listenfd);

    printf("Сервер был завершен.\n");

    return 0;
}