#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
    char method[8];
    char path[256];
} HttpRequest;

int http_parse_request_line(const char *buf, size_t len, HttpRequest *out);

#endif