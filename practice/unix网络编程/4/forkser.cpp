#include<iostream>
#include<string.h>
#include"head.hpp"

int main()
{
    int serv_fd, cli_fd;
    sockaddr_in serv, cli;
    if( serv_fd = socket(AF_INET, SOCK_STREAM, 0) < 0)
        my_err("socket", __LINE__);
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(1000);

    socklen_t len = sizeof(serv);

    if( bind(serv_fd, (struct sockaddr*)&serv, len ) < 0)
        my_err("bind", __LINE__);
    
    if(listen(serv_fd, 6) < 0)
        my_err("listen", __LINE__);

    socklen_t len1 = sizeof(cli);
    
    pid_t pid;

    while(1)
    {
        cli_fd = accept(serv_fd, (struct sockaddr*)&cli, &len1);
        if(pid = fork() == 0)
        {
            close(serv_fd);
            //此处执行exec函数族用来处理这个连接

            close(cli_fd);
            exit(0);
        }
        close(cli_fd);
    }
}