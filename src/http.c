#include "../include/http.h"

#include <stdio.h>
#include <string.h>

int http_parse_request_line(const char *buf, size_t len, HttpRequest *out)
{
    if (!buf || !out || len == 0) return -1;

    const char *eol = strstr(buf, "\r\n");
    if (!eol) return -1;

    size_t line_len = (size_t)(eol - buf);
    if (line_len == 0 || line_len >= 512) return -1;

    char line[512];
    memcpy(line, buf, line_len);
    line[line_len] = '\0';

    char version[16] = {0};
    int ok = sscanf(line, "%7s %255s %15s", out->method, out->path, version);
    return (ok == 3) ? 0 : -1;
}