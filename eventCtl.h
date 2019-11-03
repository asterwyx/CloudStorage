//
// Created by asterwyx on 11/3/19.
//

#ifndef CLOUDSTORAGESERVER_EVENTCTL_H
#define CLOUDSTORAGESERVER_EVENTCTL_H

#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "http.h"


#define MAX_EVENTS 1024  // epoll的最大数量定义为100
#define SERV_PORT 8888
#define BACKLOG 100
#define NAME_SIZE 128
#define POOL_NUM 10


typedef struct sockaddr_in sockaddr_in_t;
typedef struct epoll_event epoll_event_t;
typedef struct my_event{
    int fd;
    int events;
    void *arg;
    void (*call_back)(int fd, int events, void *arg);
    int status;
    long last_active;
    void *r_ptr;
} my_event_t;

extern int g_efd;
extern my_event_t g_events[MAX_EVENTS + 1];
void init_listen_socket(int epfd, unsigned short port);
void event_set(my_event_t *my_event, int fd, void (*call_back)(int ,int ,void *),void *arg);
void event_add(int epfd, int events, my_event_t* my_event);
void event_del(int epfd, my_event_t *my_event);
void accept_connection(int lfd, int events, void *arg);
void service(int fd, int events, void *arg);



#endif //CLOUDSTORAGESERVER_EVENTCTL_H
