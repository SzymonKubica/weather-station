#ifndef HTTP_GET_H
#define HTTP_GET_H

void http_request_task(void *request);

struct Request {
    char *web_server;
    char *web_port;
    char *web_path;
    char *body;
    int max_attempts;
};

#endif
