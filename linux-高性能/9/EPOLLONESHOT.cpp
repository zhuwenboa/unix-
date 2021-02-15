#include"head.hpp"
#include<vector>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024

struct fds 
{
    int epollfd;
    int sockfd;
};

/*设置为非阻塞*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/*将fd上的EPOLLIN和EPOLLET事件注册到epollfd指示的epoll内核事件表中，参数oneshot指定是否注册fd上的EPOLLONESHOT事件*/
void addfd(int epollfd, int  fd, bool oneshot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET; 
    if(oneshot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

/*重置fd上的事件。这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，但是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次*/
void reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void movfd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

int main()
{
    const char* ip = "127.0.0.1";
    int port = 8888;
    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setnonblocking(listenfd);
    int ret = bind(listenfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    ret = listen(listenfd, 5);

    int epollfd = epoll_create(5);
    std::vector<epoll_event> ser_event;
    ser_event.reserve(100);

    addfd(epollfd, listenfd, true);

    while(true)
    {
        int numbers = epoll_wait(epollfd, &*ser_event.begin(), 100, -1);
        for(int i = 0; i < numbers; ++i)
        {
            int fd = ser_event[i].data.fd;
            if(fd == listenfd && ser_event[i].events & EPOLLIN)
            {
                struct sockaddr_in cli_addr;
                int len = sizeof(cli_addr);
                while(true)
                {
                    int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, (socklen_t*)&len);
                    if(connfd < 0)
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            std::cout << "没有更多的连接可以建立\n";
                            break;
                        }
                        else 
                        {
                            exit(-1);
                        }
                    }
                    setnonblocking(connfd);
                    addfd(epollfd, connfd, true);
                }
            }
            else if(ser_event[i].events & EPOLLIN)
            {
                std::vector<char> buf;
                buf.reserve(1024);
                int able_recv = 1024;
                int ret;
                    while(1)
                    {
                        ret = recv(fd, &*buf.begin(), able_recv, 0);
                        if(ret == 0)
                        {
                            movfd(epollfd, fd);
                            std::cout << "foreiner closed the connection\n";
                            break;
                        }
                        else if(ret < 0)
                        {
                            if(errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                std::cout << "即将休眠\n";
                                sleep(5);
                                reset_oneshot(epollfd, fd);
                                std::cout << "read later\n";
                                break;
                            }
                            else
                            {
                                std::cout << "recv failed\n";
                                /*模拟数据处理过程*/
                                break;
                            }           
                        }
                         able_recv -= ret;
                    }
                    std::cout << "read buf = " << buf[0] << "\n";
                    std::cout << "end thread receving data on fd: " << fd << "\n";
            }
        }
    }
}

