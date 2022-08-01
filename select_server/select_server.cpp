#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "open_listenfd.h"
#include "echo.h"

#define BUF_LEN 1024 /* read buffer in bytes */
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
    int nfds, max_fd = listenfd;
    fd_set readfds, allfds; /*文件描述符集合，给select函数传递数据*/
    FD_ZERO(&allfds);
    FD_SET(listenfd, &allfds);

    /*开始监测请求*/
    for (int i;;)
    {
        readfds = allfds;
        nfds = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        for (i = 0; i <= max_fd && nfds > 0; i++)
        {
            if (FD_ISSET(listenfd, &readfds))
            {
                nfds--;
                connfd = accept(listenfd, (sockaddr *)&clientaddr, &clientlen);
                if (connfd > 0)
                {
                    FD_SET(connfd, &allfds);
                    max_fd = max_fd >= connfd ? max_fd : connfd;
                    getnameinfo((sockaddr *)&clientaddr, clientlen, client_hostname,
                                MAX_LINE, client_port, MAX_LINE, 0);
                    printf("<connect to %s:%s>\n", client_hostname, client_port);
                }
            }
            else if (FD_ISSET(i, &readfds))
            {
                nfds--;
                echo(i);
                printf("<end connection>\n");
                FD_CLR(i, &allfds);
                close(i);
                max_fd = max_fd == i ? max_fd - 1 : max_fd;
            }
        }
    }
    return 0;
}