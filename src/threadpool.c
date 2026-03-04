#include "../include/threadpool.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct {
    JobFn fn;
    void *arg;
} Job;

struct ThreadPool {
    pthread_t *threads;
    int nthreads;

    Job *q;
    size_t cap;
    size_t head;
    size_t tail;
    size_t size;

    int shutting_down;

    pthread_mutex_t mu;
    pthread_cond_t cv_has_work;
    pthread_cond_t cv_has_space;
};

static void *worker_loop(void *p)
{
    ThreadPool *tp = (ThreadPool *)p;

    for (;;) {
        pthread_mutex_lock(&tp->mu);

        while (tp->size == 0 && !tp->shutting_down) {
            pthread_cond_wait(&tp->cv_has_work, &tp->mu);
        }

        if (tp->shutting_down) {
            pthread_mutex_unlock(&tp->mu);
            break;
        }

        Job job = tp->q[tp->head];
        tp->head = (tp->head + 1) % tp->cap;
        tp->size--;

        pthread_cond_signal(&tp->cv_has_space);
        pthread_mutex_unlock(&tp->mu);

        job.fn(job.arg);
    }

    return NULL;
}

ThreadPool *tp_create(int num_threads, size_t queue_cap)
{
    if (num_threads <= 0) num_threads = 4;
    if (queue_cap < 8) queue_cap = 64;

    ThreadPool *tp = (ThreadPool *)calloc(1, sizeof(ThreadPool));
    if (!tp) return NULL;

    tp->nthreads = num_threads;
    tp->cap = queue_cap;

    tp->threads = (pthread_t *)calloc((size_t)tp->nthreads, sizeof(pthread_t));
    tp->q = (Job *)calloc(tp->cap, sizeof(Job));

    if (!tp->threads || !tp->q) {
        free(tp->threads);
        free(tp->q);
        free(tp);
        return NULL;
    }

    pthread_mutex_init(&tp->mu, NULL);
    pthread_cond_init(&tp->cv_has_work, NULL);
    pthread_cond_init(&tp->cv_has_space, NULL);

    for (int i = 0; i < tp->nthreads; i++) {
        pthread_create(&tp->threads[i], NULL, worker_loop, tp);
    }

    return tp;
}

void tp_destroy(ThreadPool *tp)
{
    if (!tp) return;

    pthread_mutex_lock(&tp->mu);
    tp->shutting_down = 1;
    pthread_cond_broadcast(&tp->cv_has_work);
    pthread_cond_broadcast(&tp->cv_has_space);
    pthread_mutex_unlock(&tp->mu);

    for (int i = 0; i < tp->nthreads; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&tp->mu);
    pthread_cond_destroy(&tp->cv_has_work);
    pthread_cond_destroy(&tp->cv_has_space);

    free(tp->threads);
    free(tp->q);
    free(tp);
}

int tp_submit(ThreadPool *tp, JobFn fn, void *arg)
{
    if (!tp || !fn) return -1;

    pthread_mutex_lock(&tp->mu);

    if (tp->shutting_down) {
        pthread_mutex_unlock(&tp->mu);
        return -1;
    }

    if (tp->size == tp->cap) {
        pthread_mutex_unlock(&tp->mu);
        return -1; // full
    }

    tp->q[tp->tail].fn = fn;
    tp->q[tp->tail].arg = arg;
    tp->tail = (tp->tail + 1) % tp->cap;
    tp->size++;

    pthread_cond_signal(&tp->cv_has_work);
    pthread_mutex_unlock(&tp->mu);

    return 0;
}