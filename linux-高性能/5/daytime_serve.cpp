#include<iostream>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include"my_err.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
        my_err("list", __LINE__);
    char *host = argv[1];
    /*获取目标主机地址信息*/
    struct hostent* hostinfo = gethostbyname(host);

    /*获取daytime服务信息*/
    struct servent* serveinfo = getservbyname("daytime", "tcp");
    printf("daytime port is %d\n", ntohs(serveinfo->s_port));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = serveinfo->s_port;

    /*注意下面的代码。因为h_addr_list本身是使用网络字节序的地址列表，所以使用其中的IP地址时，无需对目标IP地址转换字节序*/
    addr.sin_addr = *(struct in_addr*)*hostinfo->h_addr_list;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr) );

    char buffer[128];
    result = read(sockfd, buffer, sizeof(buffer));
    buffer[result] = '\0';
    printf("the day time is: %s\n", buffer);
    close(sockfd);
    return 0;


}