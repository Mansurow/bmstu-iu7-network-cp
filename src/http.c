#include "http.h"

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
    }
    else if (!strcmp(method, "HEAD"))
    {
        req = HEAD;
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

    init_header(&header);

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

    // printf("len: %d\n", path_len);
    // printf("spaces: %d\n", count_space);

    char *header_path = (char *) calloc(path_len + 1, sizeof(char));
    
    if (sscanf(buff, "%s %s HTTP/%s", method_buff, header_path, vesrion_buff) == 3)
    {

        header.request = convert_to_request_type(method_buff);
        if (header.request != UNKNOWN)
        {
            header.version = convert_to_version_type(vesrion_buff);
            
            if (!is_avaliable_path(header_path)) 
            {
                header.response = FORBIDDEN;
            }
            else 
            {
                char *path = (char *) calloc(path_len + 12, sizeof(char));
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

                header.file_type = convert_to_file_type(header.path);
            }
        } 
        else
        {
            header.response = METHOD_NOT_ALLOWED;
        }
    }  

    
    // printf("FILE TYPE: %d\n", header.file_type);
    // printf("METHOD: %d\n", header.request);
    // printf("VESION: %d\n", header.version);
    // printf("PATH: %s\n", header.path);
    // printf("RESPONSE: %d\n", header.response);

    free(header_path);

    return header;
}

char *get_content_type_header(file_type_t ft)
{
    char *response_header = (char *) calloc(HTTP_200_LEN + 53, sizeof(char));
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
			//sprintf(response_header, "%sapplication/octet-stream\r\n\r\n", HTTP_200);
            sprintf(response_header, "%stext/html; charset=utf-8\r\n\r\n", HTTP_200);
			break;
	}

    return response_header;
}
