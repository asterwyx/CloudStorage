//
// Created by asterwyx on 11/3/19.
//

#ifndef CLOUDSTORAGESERVER_THREADPOOL_H
#define CLOUDSTORAGESERVER_THREADPOOL_H

#include "condition.h"

typedef struct task{
    void (*run) (void *arg);
    void *arg;
    struct task *next;
} task_t;

typedef struct threadpool{
    condition_t ready;
    task_t *first;
    task_t *last;
    int counter;
    int idle;
    int max_threads;
    int quit;
} threadpool_t;

int threadpool_init(threadpool_t *threadpool, int threads);

void threadpool_add_task(threadpool_t *threadpool, void (*run) (void *arg), void  *arg);

void threadpool_destroy(threadpool_t *threadpool);




#endif //CLOUDSTORAGESERVER_THREADPOOL_H
