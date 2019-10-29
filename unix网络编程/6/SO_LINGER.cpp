#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<array>
#include<string.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<poll.h>
#include<limits.h>
#include<signal.h>
#include<arpa/inet.h>

int main(int argc, char *argv[])
{
    int connfd; 
    struct sockaddr_in cli_addr;
    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(atoi(argv[2]));
    cli_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //SO_LINGER选项
    struct linger cli_liger;
    cli_liger.l_onoff = 1;
    cli_liger.l_linger = 1;

    connfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(connfd, (sockaddr*)&cli_addr, sizeof(cli_addr));
    if(ret < 0)
    {
        std::cout << "connect failed\n";
        std::cout << "errno: " << ret << std::endl;
        return -1;
    }
    
    setsockopt(connfd, SOL_SOCKET, SO_LINGER, &cli_liger, sizeof(cli_liger));

    std::cout << "connect sunccessfully\n";
    
    ret = close(connfd);
    if(ret < 0)
    {
        if(errno == EWOULDBLOCK)
        {
            std::cout << "服务器发来了ack\n";
        }
        else
        {
            std::cout << "----\n";
        }
        
    }

    /*
    用shutdown函数来关闭写端，用read函数等待直到服务端发送了ack和fin后返回
    //关闭写端
    shutdown(connfd, SHUT_WR);
    int flag;
    ret = read(connfd, (char*)&flag, 1);
    if(ret == 0)
    {
        //等待直到收到服务端的FIN和ack后返回
        std::cout << "close is already\n";
    }
    else
    {
        std::cout << ".....\n";   
    }
    */   
}