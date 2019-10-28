#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<array>
#include<string.h>
#include<sys/time.h>
#include<netinet/in.h>

using namespace std;
#define max_con 64
#define port 6666
static int count = 0;   //当前sel_fd中的数目

array<int, 64>sel_fd;  //已连接fd的集合

using namespace std;

//设置为非阻塞
void setnonblock(const int &fd)
{
    int old_op = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_op | O_NONBLOCK);
}

int main(int argc, char *argv[])
{
    fd_set fd_read, fd_write;
    std::cout << sizeof(fd_set) << endl; //最大可连接描述符数量

    struct sockaddr_in ser_addr, cli_addr;
    int listenfd;
    //客户连接
    int connfd;
    memset(&ser_addr, 0, sizeof(ser_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;
    //accept中用到
    socklen_t cli_len = sizeof(cli_addr);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        cout << "socket failed\n";
        return -1;
    }
    setnonblock(listenfd);
    int ret = bind(listenfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    if(ret < 0)
    {
        cout << "bind failed\n";
        return -1;
    }
    ret = listen(listenfd, 5);
    if(ret < 0)
    {
        cout << "listen failed\n";
        return -1;
    }
    struct timeval sel_time;
    sel_time.tv_sec = 20;
    sel_time.tv_usec = 0;
    FD_ZERO(&fd_read);
    //FD_ZERO(&fd_write);
    FD_SET(listenfd, &fd_read);
    int max_fd = listenfd;
 //   sel_fd.fill(listenfd);
 //   ++count;
    char readbuf[1024] = {0};
    char sendbuf[1024] = {0};
    while(true)
    {
        std::cout << "select wait\n";
        ret = select(max_fd + 1, &fd_read, NULL, NULL, &sel_time);
        if(ret < 0)
        {
            cout << "select erro\n";
            for(int i = 0; i < count; ++i)
                close(sel_fd[i]);
            return -1;
        }
        if(ret == 0)
        {
            cout << "select timeout \n";
            break;
        }
        for(int i = 0; i < ret; ++i)
        {
            //有新的连接
            if(FD_ISSET(listenfd, &fd_read))
            {
                std::cout <<"有新的连接到来\n";
                connfd = accept(listenfd, (sockaddr*)&cli_addr, &cli_len);
                setnonblock(connfd);
                //加入到已连接事件集中
                if(count < max_con)
                {
                    sel_fd[count] = connfd;
                    ++count; 
                    if(connfd > max_fd)
                        max_fd = connfd;
                }
                else
                {
                    char *str = "max connection, sorry";
                    send(connfd, str, strlen(str) + 1, 0);
                    close(connfd);
                }
            }   
            for(int i = 0; i < count; ++i)
            {
                if(FD_ISSET(sel_fd[i], &fd_read))
                {
                    ret = recv(sel_fd[i], readbuf, 1023, 0);
                    if(ret < 0)
                    {
                        std::cout << "recv failed\n";
                    }
                    //有连接断开
                    if(ret == 0)
                    {
                        std::cout << "用户"  << sel_fd[i] << "断开连接\n";
                        FD_CLR(sel_fd[i], &fd_read);
                        close(sel_fd[i]);
                        sel_fd[i] = -1;
                    }   
                    else
                    {
                        printf("recv %d's data : %s", sel_fd[i], readbuf);
                    }
                    memset(readbuf, 0, sizeof(readbuf));
                }
            }
            //每次select轮询都得更新fd_set
            FD_ZERO(&fd_read);
            FD_SET(listenfd, &fd_read);
            for(int i = 0; i < count; ++i)
            {
                FD_SET(sel_fd[i], &fd_read);
            }
            sel_time.tv_sec = 5; //因为每次select返回，如果不更新定时时间，则每次都是之前的tv_sec减去等待的时间。
        }
    }
    close(listenfd);
}