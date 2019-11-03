//
// Created by asterwyx on 10/29/19.
//

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 1024
#define MAX_EVENTS 1024

typedef struct efddata{
    void * ptr;
    int fd;
    uint64_t len;
} efd_data;

int main(int argc,char* argv[]){
    int sfd,cfd,epfd,nfds,n,r,w;
    char* buf[MAX_EVENTS];
    struct sockaddr_in addr_in = {0};

    // 创建epoll
    epfd = epoll_create1(EPOLL_CLOEXEC);
    printf("epfd: %d\n",epfd);
    if (epfd == -1){
        perror("epoll_create1 fail");
        exit(EXIT_FAILURE);
    }

    // 创建监听socket
    sfd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    printf("sfd: %d\n",sfd);
    if (sfd == -1){
        perror("socket fail");
        exit(EXIT_FAILURE);
    }

    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(8888);
    if(bind(sfd,(const struct sockaddr*)&addr_in,sizeof(struct sockaddr_in)) == -1){
        perror("bind fail");
        exit(EXIT_FAILURE);
    }

    if(listen(sfd,LISTEN_BACKLOG) == -1){
        perror("listen fail");
        exit(EXIT_FAILURE);
    }

    // 将监听socket的EPOLLIN事件加入队列
    struct epoll_event evt = {0};
    evt.events = EPOLLIN;
    evt.data.fd = sfd;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&evt) == -1){
        perror("epoll_ctl add epfd fail");
        exit(EXIT_FAILURE);
    }

    // 预分配fd事件内存
    struct epoll_event events[MAX_EVENTS],*fdevents[MAX_EVENTS];
    for(n = 0; n < MAX_EVENTS; ++n){
        struct epoll_event *e = (struct epoll_event *)malloc(sizeof(struct epoll_event));
        if(e == NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        fdevents[n] = e;
    }

    for(;;){
        nfds = epoll_wait(epfd,events,MAX_EVENTS,-1);
        if(nfds == -1){
            perror("epoll_wait fail");
            close(epfd);
            exit(EXIT_FAILURE);
        }

        for(n = 0; n < nfds; ++n){
            if(events[n].data.fd == sfd){ // 接受客户端连接
                cfd = accept(sfd,NULL,NULL);
                if (cfd == -1){
                    fprintf(stderr,"accept fail %d\n",sfd);
                    continue;
                }

                char * rbuf = (char *)malloc(BUFFER_SIZE);
                buf[cfd] = rbuf;
                efd_data* data = (efd_data *)malloc(sizeof(efd_data));
                data->ptr = buf[cfd];
                data->fd = cfd;
                struct epoll_event *e = fdevents[cfd];
                e->events = EPOLLIN;
                e->data.ptr = data;
                if(epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,fdevents[cfd]) == -1){
                    perror("epoll_ctl add fail");
                    close(cfd);
                }
                fprintf(stdout,"accept cfd %d\n",cfd);
            }else{
                efd_data * data = (efd_data *)events[n].data.ptr;

                // 处理读事件
                if(events[n].events & EPOLLIN){
                    fprintf(stdout,"epollin ready cfd %d\n",data->fd);

                    r = read(data->fd,data->ptr,BUFFER_SIZE);
                    if(r == -1){
                        perror("read fail");
                        if(epoll_ctl(epfd,EPOLL_CTL_DEL,data->fd,NULL) == -1){
                            perror("epoll_ctl del fail");
                            exit(EXIT_FAILURE);;
                        }
                        close(data->fd);
                    }else{
                        struct epoll_event *evt = fdevents[data->fd];
                        evt->events = EPOLLOUT;
                        ((efd_data *)(evt->data.ptr))->len= r;
                        if(epoll_ctl(epfd,EPOLL_CTL_MOD,data->fd,evt) == -1){
                            perror("epoll_ctl mod epollout fail");
                        }
                    }
                }

                // 处理写事件
                if(events[n].events & EPOLLOUT){
                    w = write(data->fd,data->ptr,data->len);
                    if(w == -1){
                        fprintf(stderr,"write fail to cfd:%d\n",data->fd);
                    }
                    epoll_ctl(epfd,EPOLL_CTL_DEL,data->fd,NULL);
                    close(data->fd);
                    fprintf(stdout,"write success cfd: %d\n",data->fd);
                }
            }
        }

    }


}