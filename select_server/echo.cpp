#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
const int max_size = 60000;
void echo(int connfd)
{
    /*buf数组获取请求报文，senddata数组存储响应报文
    这里不管请求报文是什么，都会发送固定的响应报文*/
    char *buf = new char[max_size], *senddata = new char[max_size];
    FILE *fp1 = fopen("../answer.txt", "r"), *fp2 = fopen("../index.html", "r");
    int len = fread(senddata, 1, max_size, fp1);
    len += fread(senddata + len, 1, max_size, fp2);
    
    /*接收并展示请求报文，发送响应报文*/
    if (recv(connfd, buf, max_size, 0) > 0)
    {
        printf("%s\n", buf);
        send(connfd, senddata, len, 0);
    }
    
    fclose(fp1);
    fclose(fp2);
}
