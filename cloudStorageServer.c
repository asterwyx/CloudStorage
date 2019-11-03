#include "http.h"

int main(int argc, char *argv[])
{
    //设置端口
    unsigned short port=SERV_PORT;

    if(argc == 2)
        port=atoi(argv[1]);  //如果命令行输入了端口，就按输入的端口来

    g_efd = epoll_create(MAX_EVENTS+1);
    if(g_efd < 0)
        printf("Create epfd error:%s", strerror(errno));
    else{
        printf("g_efd:%d", g_efd);
    }

    init_listen_socket(g_efd, port);  //初始化监听socket

    init();

    epoll_event_t events[MAX_EVENTS + 1]; // epoll_event缓冲区
    memset(events, 0, (MAX_EVENTS + 1) * sizeof(struct epoll_event));  // 初始化缓冲区

    // 记录激活的事件数
    int triggered_num = 0;

    // 遍历epoll_event缓冲区
    while (1) {
        long now = time(NULL);
        triggered_num = epoll_wait(g_efd, events, MAX_EVENTS + 1, -1);
        if (triggered_num < 0) {
            perror("epoll_wait error!\n");
            break;
        }
        for (int i = 0; i < triggered_num; i++) {
            my_event_t *event = (my_event_t *)events[i].data.ptr;

            if((events[i].events & EPOLLIN) && (event->events & EPOLLIN))
                event->call_back(event->fd,events[i].events,event->arg);
        }
    }
    close(g_efd);
    return 0;
}




	

	
	


	
