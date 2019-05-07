#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<assert.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; //ipv4
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock_fd > 0);

    if(bind(sock_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        fprintf(stderr, "bind erro: %d\n", errno);
    if(listen(sock_fd, 3) < 0)
        fprintf(stderr, "listen erro:%d\n", errno);
    sleep(20);    
    struct sockaddr_in client;
    socklen_t client_addrlen = sizeof(client);
    int connfd = accept(sock_fd, (struct sockaddr *)&client, &client_addrlen);
    if(connfd < 0)
        printf("errno is: %d\n", errno);
    else
    {
        //接受连接成功则打印出客户端的IP地址和端口号
        char remote[INET_ADDRSTRLEN];
        printf("connected with ip: %s and port: %d\n", inet_ntop(AF_INET, &client_addrlen, remote, INET_ADDRSTRLEN),
        ntohs(client.sin_port)) ;
        close(connfd);
    }
    close(sock_fd);
    return 0;

}