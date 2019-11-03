//
// Created by asterwyx on 11/3/19.
//
#include "eventCtl.h"

int g_efd;
my_event_t g_events[MAX_EVENTS + 1];


void init_listen_socket(int epfd, unsigned short port) {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Listen socket descriptor:%d\n", lfd);

    fcntl(lfd, F_SETFL, O_NONBLOCK);

    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse, sizeof(reuse));
    sockaddr_in_t serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port=htons(port);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(	bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0)
        perror("bind error:");
    else
        printf("bind ok!\n");
    // 设置监听数量
    if(listen(lfd, BACKLOG)<0)
        perror("listen error:");
    else
        printf("listen ok!\n");

    //把监听的lfd（只有一个）设置在g_events的最后一个元素中 g_events[MAX_EVENTS]
    // g_events是自定的事件数组，存储事件
    // 这个指向自己的泛型参数有什么用？
    event_set(&g_events[MAX_EVENTS],lfd,accept_connection,&g_events[MAX_EVENTS]);

    //将 g_events[MAX_EVENTS] 加入到efd，事件为EPOLLIN
    // 用来把事件数组上的事件放入epoll监听，并设置监听类型
    event_add(epfd,EPOLLIN,&g_events[MAX_EVENTS]);
}

void event_set(my_event_t *my_event, int fd, void (*call_back)(int ,int ,void *),void *arg) {
    my_event->fd = fd;
    my_event->events = 0;
    my_event->call_back = call_back;
    my_event->arg = arg;
    my_event->status = 0;
    my_event->last_active = time(NULL);
    my_event->r_ptr = NULL;
}


void event_add(int epfd, int events, my_event_t* my_event) {
    epoll_event_t *epv = (epoll_event_t *)malloc(sizeof(epoll_event_t));
    int op;
    epv->data.ptr = my_event;
    epv->events = my_event->events = events;

    if(my_event->status==1){
        op=EPOLL_CTL_MOD;

    }else{
        op=EPOLL_CTL_ADD;
        my_event->status=1;
    }

    if(epoll_ctl(g_efd, op, my_event->fd, epv)<0){
        printf("Event add failed! \t fd:%d  events:%d",my_event->fd,events);
    }else{
        printf("Event add successfully! \t efd:%d  fd:%d  events:%d\n", epfd, my_event->fd, events);
    }
}

void event_del(int epfd, my_event_t *my_event) {
    struct epoll_event epv = {0,{0}};
    if(my_event->status != 1)
        return;
    epv.data.ptr = my_event;
    my_event->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, my_event->fd, &epv);
}

void accept_connection(int lfd, int events, void *arg) {
    sockaddr_in_t client_addr;
    socklen_t  len = sizeof(client_addr);

    int acceptfd,i;

    if((acceptfd=accept(lfd,(struct sockaddr*)&client_addr,&len))==-1){
        perror("accept error:");
        return;
    }else{
        printf("Accept a new client:%s,%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
    }

    do {
        for(i = 0; i < MAX_EVENTS; i++){		//找到某个不在树上的g_events[i]
            if(g_events[i].status == 0)
                break;
        }
        if(i == MAX_EVENTS){
            printf("cannot accept more client!\n");
            break;
        }

        int flag = 0;
        if((flag = fcntl(acceptfd, F_SETFL, O_NONBLOCK))<0){
            printf("fcntl nonblock failed:%s\n ",strerror(errno));
            break;
        }
        //	void event_set(struct my_event* ev,int fd,void (*call_back)(int ,int ,void *),void *arg)
        event_set(&g_events[i], acceptfd, service, &g_events[i]);   //将事件注册到这个新连接的cfd上的
        //  void event_add(int efd,int events,struct my_event* ev)
        event_add(g_efd,EPOLLIN,&g_events[i]);				//将g_events[i].fd（也就是cfd）添加到g_efd中,让g_efd来管理 g_events[i].fd

    } while(0);
}

