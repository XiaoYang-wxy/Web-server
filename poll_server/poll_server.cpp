#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include "open_listenfd.h"
#include "echo.h"

#define MAX_SIZE 2048
#define MAX_LINE 1024
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
    int nfds, max_index = 0;
    pollfd fds[MAX_SIZE]; /*文件描述符集合，给poll函数传递数据*/
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;
    for (int i = 1; i < MAX_SIZE; i++)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

    /*开始监测请求*/
    for (int i, j, success;;)
    {
        nfds = poll(fds, max_index + 1, -1);
        if (fds[0].revents & POLLIN)
        {
            nfds--;
            connfd = accept(listenfd, (sockaddr *)&clientaddr, &clientlen);
            if (connfd > 0)
            {
                for (j = 1, success = 0; j < MAX_SIZE; j++)
                    if (fds[j].fd == -1)
                    {
                        success = 1;
                        fds[j].fd = connfd;
                        fds[j].events = POLLIN;
                        max_index = j > max_index ? j : max_index;
                        break;
                    }
                if (success == 0)
                {
                    printf("已达到最大连接数量:%d!", MAX_SIZE);
                    close(connfd);
                }
                else
                {
                    getnameinfo((sockaddr *)&clientaddr, clientlen, client_hostname,
                                MAX_LINE, client_port, MAX_LINE, 0);
                    printf("<connect to %s:%s>\n", client_hostname, client_port);
                }
            }
        }
        for (i = 1; i < MAX_SIZE && nfds > 0; i++)
            if (fds[i].revents & POLLIN)
            {
                nfds--;
                echo(fds[i].fd);
                printf("<end connection>\n");
                close(fds[i].fd);
                fds[i].fd = -1;
                max_index = max_index == i ? max_index - 1 : max_index;
            }
    }
    return 0;
}