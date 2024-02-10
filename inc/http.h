# ifndef __HTTP_H
# define __HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFFER_LEN      1024
#define MAX_GET_REQUEST 2048

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
	CSS  = 2,
	JS   = 3,
	JPG  = 4,
	JPEG = 5,
	SWF  = 6,
	GIF  = 7,
	PNG  = 8,
	TXT  = 9,
	SVG  = 10
} file_type_t;

typedef enum http_request {
    UNKNOWN = 0,
    GET     = 1,
    HEAD    = 2
} http_request_t;

typedef enum http_reponse {
    DEFAULT    = 0,
    CONTINUE   = 100,
    PROCESSING = 102,
    OK         = 200,
    NO_CONTENT = 204,
	FORBIDDEN  = 403,
	NOT_FOUND  = 404,
    CONFLICT   = 409,
    METHOD_NOT_ALLOWED = 405,
    NOT_IMPLEMENTED = 501,
} http_reponse_t;

// кидать 501 если не 1.1
typedef enum http_version {
    http1   = 1,
    http1_1 = 11,
    http2   = 2,
    http3   = 3
} http_version_t;

typedef struct http_header {
    char *path;
    http_version_t version;
    http_request_t request;
    file_type_t file_type;
    http_reponse_t response;
} http_header_t;


void init_header(http_header_t *header);

http_request_t convert_to_request_type(const char *method);
http_version_t convert_to_version_type(const char *version);
file_type_t convert_to_file_type(char *path);
char *get_content_type_header(file_type_t ft);

http_header_t parse_header(const char *buff);

# endif