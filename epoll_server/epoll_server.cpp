#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "open_listenfd.h"
#include "echo.h"

#define MAX_LINE 1024
#define SIZE 1024
char port[6] = "10613";

int main()
{
    socklen_t clientlen = sizeof(sockaddr_storage);
    sockaddr_storage clientaddr; /*enough space for any address*/
    char client_hostname[MAX_LINE], client_port[MAX_LINE];

    /*建立监听描述符*/
    int listenfd, connfd;
    if ((listenfd = open_listenfd(port)) == -1)
    {
        printf("cannot listen.\n");
        return 0;
    }

    /*初始化*/
    int epoll_fd = epoll_create(SIZE), nfds;
    epoll_event event, *events = new epoll_event[SIZE];
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &event);

    /*开始监测请求*/
    for (int i;;)
    {
        nfds = epoll_wait(epoll_fd, events, SIZE, -1);
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listenfd && (events[i].events & EPOLLIN))
            {
                connfd = accept(listenfd, (sockaddr *)&clientaddr, &clientlen);
                if (connfd > 0)
                {
                    event.data.fd = connfd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &event);
                    getnameinfo((sockaddr *)&clientaddr, clientlen, client_hostname,
                                MAX_LINE, client_port, MAX_LINE, 0);
                    printf("<connect to %s:%s>\n", client_hostname, client_port);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                echo(events[i].data.fd);
                printf("<end connection>\n");
                event.data.fd = events[i].data.fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                close(events[i].data.fd);
            }
        }
    }
}