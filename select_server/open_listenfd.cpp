#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
const int listenq = 1024;
int open_listenfd(char *port)
{
    addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    /*get a list of potential server address*/
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    /*walk the list for one that we can bind to*/
    for (p = listp; p; p = p->ai_next)
    {
        /*create a socket descriptor*/
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        /*eliminates "Address already in use" error from bind*/
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int));

        /*bind the descriptor to the address*/
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /*success*/

        close(listenfd); /*bind failed,try next*/
    }

    /*clean up*/
    freeaddrinfo(listp);
    /*no address worked*/
    if (!p)
        return -1;

    /*make it a listening socket ready to accept connection requests*/
    if (listen(listenfd, listenq) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}
