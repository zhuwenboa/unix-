#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include"my_err.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << "erro" << __LINE__ << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int sockfd;
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        my_err("socket", __LINE__);
    if( bind(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr) ) < 0)
        my_err("bind", __LINE__);
    if( listen(sockfd, 5) < 0)
        my_err("listen", __LINE__);
    
    socklen_t len = sizeof(ser_addr);
    int connfd;
    if( (connfd = accept(sockfd, (struct sockaddr*)&ser_addr, &len)) < 0)
        my_err("accept", __LINE__);
    char buffer[BUF_SIZE] = {0};
    int ret;
    //1
    if( (ret = recv(connfd, buffer, BUF_SIZE - 1, 0) ) < 0)
        my_err("recv", __LINE__);
    printf("got %d bytes of normal data '%s' \n", ret, buffer);
    //2
    memset(buffer, '\0', sizeof(buffer));
    ret = recv(connfd, buffer, BUF_SIZE -1, 0);
    if(ret < 0)
        my_err("recv", __LINE__);
    printf("got %d bytes of oob data '%s' \n", ret, buffer);
    //3
    memset(buffer, '\0', sizeof(buffer));
    ret = recv(connfd, buffer, BUF_SIZE -1, 0);
    if(ret < 0)
        my_err("recv", __LINE__);
    printf("got %d bytes of normal data '%s' \n", ret, buffer);

    close(connfd);

    close(sockfd);
}