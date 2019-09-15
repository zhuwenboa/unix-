#include"head.hpp"

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
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

/*工作线程*/
void* worker(void *arg)
{
    int sockfd = ( (fds*)arg )->sockfd;
    int epollfd = ( (fds*)arg )->epollfd;
    std::cout << "start new thread to receive data on fd :" << sockfd << "\n";
    char buf[BUFFER_SIZE] = {0};
    /*循环读取sockfd上的数据，直到遇到EAGAIN错误*/
    while(1)
    {
        int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
        if(ret == 0)
        {
            close(sockfd);
            std::cout << "foreiner closed the connection\n";
            break;
        }
        else if(ret < 0)
        {
            if(errno == EAGAIN)
            {
                reset_oneshot(epollfd, sockfd);
                std::cout << "read later\n";
                break;
            }
        }
        else
        {
            std::cout << "get content: " << buf << std::endl;
            /*模拟数据处理过程*/
            sleep(5);
        }
        
    }
    std::cout << "end thread receving data on fd: " << sockfd << "\n";
    
}

