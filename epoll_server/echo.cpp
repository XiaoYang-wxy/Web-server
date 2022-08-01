#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
const int max_size = 60000;
void echo(int connfd)
{
    char *buf = new char[max_size], *senddata = new char[max_size];
    FILE *fp1 = fopen("answer.txt", "r"), *fp2 = fopen("index.html", "r");
    int len = fread(senddata, 1, max_size, fp1);
    len += fread(senddata + len, 1, max_size, fp2);
    while (recv(connfd, buf, max_size, 0) != 0)
    {
        printf("%s\n", buf);
        send(connfd, senddata, len, 0);
        break;
    }
    fclose(fp1);
    fclose(fp2);
}