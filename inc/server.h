# ifndef __SERVER_H
# define __SERVER_H

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

#define SOCKET             "server.socket"
#define DEFAULT_ROOT_DIR   "./static"
#define PATH_BUFF_LEN      256
#define DEFAULT_PORT       8089
#define LISTEN_BACKLOG     32
#define MAX_CONNECTIONS    2000



# endif