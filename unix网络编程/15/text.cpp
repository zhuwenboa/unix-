#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<iostream>
#include<error.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/select.h>
#include<netinet/tcp.h>
#include<linux/tcp.h>

int main()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //fcntl(listenfd, F_SETFL, O_NONBLOCK);
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(serv_addr);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bind(listenfd, (sockaddr*)&serv_addr, addrlen);
    listen(listenfd, 5);

    char buffer[1024] = {0};
    int clientfd = accept(listenfd, (sockaddr*)&serv_addr, &addrlen);
    if(read(clientfd, buffer, sizeof(buffer)) == 0)
    {
        std::cout << "llll\n";
        struct tcp_info info;
        int len = sizeof(info);
        getsockopt(clientfd, IPPROTO_TCP, TCP_INFO, (void*)&info, (socklen_t*)&len);
        if(info.tcpi_state != TCP_ESTABLISHED)
        {
            std::cout << "连接断开\n";
        }
       /*
        int ret = send(clientfd, "1", 1, 0);
        std::cout << "ret = " << ret << "\n";

        if(errno == EPIPE)
        {
            std::cout << "连接断开\n";
        }
        */
    }
    return 0;
}