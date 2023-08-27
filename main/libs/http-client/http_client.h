#ifndef HTTP_GET_H
#define HTTP_GET_H

#include "json/cJSON.h"

struct Request {
    char *web_server;
    char *web_port;
    char *web_path;
    char *body;
    int max_attempts;
};

#define REQUEST_SUCCESS 0
#define REQUEST_FAILED -1
int send_http_request(struct Request *request, cJSON **response);

#endif
