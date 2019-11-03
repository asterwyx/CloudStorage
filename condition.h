//
// Created by asterwyx on 11/3/19.
//

#ifndef CLOUDSTORAGESERVER_CONDITION_H
#define CLOUDSTORAGESERVER_CONDITION_H

#include <pthread.h>

typedef struct condition {
    pthread_mutex_t pthreadMutex;
    pthread_cond_t pthreadCond;
} condition_t;

int condition_init(condition_t *condition);
int condition_lock(condition_t *condition);
int condition_unlock(condition_t *condition);
int condition_wait(condition_t *condition);
int condition_timewait(condition_t *condition, const struct timespec *abstime);
int condition_signal(condition_t *condition);
int condition_broadcast(condition_t *condition);
int condition_destroy(condition_t *condition);



#endif //CLOUDSTORAGESERVER_CONDITION_H
