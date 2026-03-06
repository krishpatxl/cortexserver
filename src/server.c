#include "../include/server.h"
#include "../include/http.h"
#include "../include/threadpool.h"
#include "../include/files.h"

#include <netinet/in.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int fd;
    char buf[4096];
    size_t len;
} RequestJob;

static atomic_long total = 0;
static atomic_long ok = 0;
static atomic_long not_found = 0;

static void reply_text(int fd, int status, const char *body)
{
    const char *msg = (status == 200) ? "200 OK" : "404 Not Found";

    char head[512];
    int body_len = (int)strlen(body);

    int n = snprintf(
        head,
        sizeof(head),
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        msg,
        body_len
    );

    send(fd, head, (size_t)n, 0);
    send(fd, body, (size_t)body_len, 0);
}

static void handle_request(void *arg)
{
    RequestJob *job = (RequestJob *)arg;

    HttpRequest r;
    if (http_parse_request_line(job->buf, job->len, &r) != 0) {
        atomic_fetch_add(&not_found, 1);
        reply_text(job->fd, 404, "bad request\n");
        close(job->fd);
        free(job);
        return;
    }

    atomic_fetch_add(&total, 1);

    if (strcmp(r.method, "GET") != 0) {
        atomic_fetch_add(&not_found, 1);
        reply_text(job->fd, 404, "only GET for now\n");
        close(job->fd);
        free(job);
        return;
    }

    if (strcmp(r.path, "/") == 0) {
        if (send_file_response(job->fd, "public/index.html") == 0) {
            atomic_fetch_add(&ok, 1);
        } else {
            atomic_fetch_add(&not_found, 1);
            reply_text(job->fd, 404, "index file not found\n");
        }

    } else if (strcmp(r.path, "/health") == 0) {
        atomic_fetch_add(&ok, 1);
        reply_text(job->fd, 200, "OK\n");

    } else if (strcmp(r.path, "/metrics") == 0) {
        atomic_fetch_add(&ok, 1);

        long t = atomic_load(&total);
        long o = atomic_load(&ok);
        long nf = atomic_load(&not_found);

        char out[256];
        snprintf(
            out,
            sizeof(out),
            "requests_total %ld\nrequests_ok %ld\nrequests_404 %ld\n",
            t,
            o,
            nf
        );

        reply_text(job->fd, 200, out);

    } else if (strncmp(r.path, "/static/", 8) == 0) {
        const char *rel = r.path + 8;

        // basic path traversal protection
        if (strstr(rel, "..") != NULL) {
            atomic_fetch_add(&not_found, 1);
            reply_text(job->fd, 404, "invalid path\n");
        } else {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "public/%s", rel);

            if (send_file_response(job->fd, full_path) == 0) {
                atomic_fetch_add(&ok, 1);
            } else {
                atomic_fetch_add(&not_found, 1);
                reply_text(job->fd, 404, "file not found\n");
            }
        }

    } else {
        atomic_fetch_add(&not_found, 1);
        reply_text(job->fd, 404, "not found\n");
    }

    close(job->fd);
    free(job);
}

int start_server(int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return -1;
    }

    if (listen(s, 128) < 0) {
        perror("listen");
        close(s);
        return -1;
    }

    ThreadPool *tp = tp_create(4, 256);
    if (!tp) {
        fprintf(stderr, "threadpool init failed\n");
        close(s);
        return -1;
    }

    printf("CortexServer listening on port %d (thread pool enabled)\n", port);

    for (;;) {
        int c = accept(s, NULL, NULL);
        if (c < 0) {
            perror("accept");
            continue;
        }

        char buf[4096];
        ssize_t n = recv(c, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(c);
            continue;
        }
        buf[n] = '\0';

        RequestJob *job = (RequestJob *)calloc(1, sizeof(RequestJob));
        if (!job) {
            reply_text(c, 404, "server error\n");
            close(c);
            continue;
        }

        job->fd = c;
        job->len = (size_t)n;
        memcpy(job->buf, buf, (size_t)n + 1);

        if (tp_submit(tp, handle_request, job) != 0) {
            reply_text(c, 404, "server busy\n");
            close(c);
            free(job);
        }
    }

    tp_destroy(tp);
    close(s);
    return 0;
}