//
// Created by asterwyx on 11/3/19.
//

#include "condition.h"

int condition_init(condition_t *condition) {
    int status;
    if ((status = pthread_mutex_init(&condition->pthreadMutex, NULL)) != 0) {
        return status;
    }
    if ((status = pthread_cond_init(&condition->pthreadCond, NULL)) != 0) {
        return status;
    }
    return 0;
}

int condition_lock(condition_t *condition) {
    return pthread_mutex_lock(&condition->pthreadMutex);
}

int condition_unlock(condition_t *condition) {
    return pthread_mutex_unlock(&condition->pthreadMutex);
}

int condition_wait(condition_t *condition) {
    return pthread_cond_wait(&condition->pthreadCond, &condition->pthreadMutex);
}

int condition_timewait(condition_t *condition, const struct timespec *abstime) {
    return pthread_cond_timedwait(&condition->pthreadCond, &condition->pthreadMutex, abstime);
}

int condition_signal(condition_t *condition) {
    return pthread_cond_signal(&condition->pthreadCond);
}

int condition_broadcast(condition_t *condition) {
    return pthread_cond_broadcast(&condition->pthreadCond);
}

int condition_destroy(condition_t *condition) {
    int status;
    if ((status = pthread_mutex_destroy(&condition->pthreadMutex))) {
        return status;
    }
    if ((status = pthread_cond_destroy(&condition->pthreadCond))) {
        return status;
    }
    return 0;
}
