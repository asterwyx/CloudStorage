//
// Created by asterwyx on 11/3/19.
//

#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void *thread_routine(void *arg)
{
    struct timespec abstime;
    int timeout;
    threadpool_t *threadpool = (threadpool_t *)arg;
    printf("Thread %lu is starting...\t\tpoolsize:%d\n", pthread_self(), threadpool->max_threads);
    while (1) {
        timeout = 0;
        condition_lock(&threadpool->ready);
        threadpool->idle++;
        while (threadpool->first == NULL && !threadpool->quit) {
            printf("Thread %lu is waiting...\t\tpoolsize:%d\n", pthread_self(), threadpool->max_threads);
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += 10;
            int status;
            status = condition_timewait(&threadpool->ready, &abstime);
            if (status == ETIMEDOUT) {
                printf("Thread %lu wait time out.\n", pthread_self());
                timeout = 1;
                break;
            }
        }

        threadpool->idle--;
        if (threadpool->first != NULL) {
            task_t *t = threadpool->first;
            threadpool->first = t->next;
            condition_unlock(&threadpool->ready);
            t->run(t->arg);
            free(t);
            condition_lock(&threadpool->ready);
        }

        if (threadpool->quit && threadpool->first == NULL) {
            threadpool->counter--;
            if (threadpool->counter == 0) {
                condition_signal(&threadpool->ready);
            }
            condition_unlock(&threadpool->ready);
            break;
        }
        if (timeout == 1) {
            threadpool->counter--;
            condition_unlock(&threadpool->ready);
            break;
        }
        condition_unlock(&threadpool->ready);
    }
    printf("Thread %lu is exiting...\n", pthread_self());
    return NULL;
}

int threadpool_init(threadpool_t *threadpool, int threads) {
    int status;
    status = condition_init(&threadpool->ready);
    threadpool->first = NULL;
    threadpool->last = NULL;
    threadpool->counter = 0;
    threadpool->idle = 0;
    threadpool->max_threads = threads;
    threadpool->quit = 0;
    return status;
}

void threadpool_add_task(threadpool_t *threadpool, void (*run)(void *), void *arg) {
    task_t *newtask = (task_t *)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->arg = arg;
    newtask->next = NULL;

    condition_lock(&threadpool->ready);

    if (threadpool->first == NULL) {
        threadpool->first = newtask;
    } else {
        threadpool->last = newtask;
    }
    threadpool->last = newtask;

    if (threadpool->idle > 0) {
        condition_signal(&threadpool->ready);
    } else if (threadpool->counter < threadpool->max_threads) {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, threadpool);
        threadpool->counter++;
    }

    condition_unlock(&threadpool->ready);
}

void threadpool_destroy(threadpool_t *threadpool) {
    if (threadpool->quit) {
        return ;
    }

    condition_lock(&threadpool->ready);
    threadpool->quit = 1;
    if (threadpool->counter > 0) {
        if (threadpool->idle > 0) {
            condition_broadcast(&threadpool->ready);
        }

        while (threadpool->counter) {
            condition_wait(&threadpool->ready);
        }
    }
    condition_unlock(&threadpool->ready);
    condition_destroy(&threadpool->ready);
}
