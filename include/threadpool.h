#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>

typedef struct ThreadPool ThreadPool;

typedef void (*JobFn)(void *arg);

ThreadPool *tp_create(int num_threads, size_t queue_cap);
void tp_destroy(ThreadPool *tp);

// returns 0 on success, -1 if queue is full or shutting down
int tp_submit(ThreadPool *tp, JobFn fn, void *arg);

#endif