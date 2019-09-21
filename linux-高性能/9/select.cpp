#include<iostream>
#include"head.hpp"

int main(int argc, char **argv)
{
    if(argc <= 2)
        my_err("list", __LINE__);
    const char *ip  = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM ,0);
    if(listenfd < 0)
        my_err("socket", __LINE__);
    
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    if(ret < 0)
        my_err("bind", __LINE__);
    
    ret = listen(listenfd, 5);
    if(ret < 0)
        my_err("listen", __LINE__);
    
    struct sockaddr_in cli_address;
    socklen_t cli_len  = sizeof(cli_address);
    int connectfd = accept(listenfd, (struct sockaddr*)&cli_address, &cli_len);
    if(connectfd < 0)
        my_err("connectfd", __LINE__);
    
    char buf[1024];
    fd_set read_fds;
    fd_set exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);

    while(1)
    {
        memset(buf, '\0', sizeof(buf));
        /*每次调用select前都要重新在read_fds和execption_fds中设置文件描述符connfd,因为事件发生之后，文件描述符集合将被内核修改*/
        FD_SET(connectfd, &read_fds);
        FD_SET(connectfd, &exception_fds);

        ret = select(connectfd + 1, &read_fds, NULL, &exception_fds, NULL);
        if(ret < 0)
            my_err("select", __LINE__);
        /*对于可读事件，采用普通的recv函数读取数据*/
        if(FD_ISSET(connectfd, &read_fds) )
        {
            ret = recv(connectfd, buf, sizeof(buf) - 1, 0);
            if(ret < 0)
                my_err("recv", __LINE__);
            std::cout << "get" << ret << "bytes of normal data: " << buf << std::endl;
        }
        else if(FD_ISSET(connectfd, &exception_fds) )
        {
            ret = recv(connectfd, buf, sizeof(buf) - 1, MSG_OOB);
            if(ret < 0)
                my_err("recv msg_oob", __LINE__);
            std::cout << "get" << ret << "bytes of oob data: " << buf << std::endl;
        }
        close(connectfd);
        close(listenfd);
        return 0;
        


    }
    
}