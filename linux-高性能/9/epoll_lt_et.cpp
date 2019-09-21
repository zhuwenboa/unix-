#include"head.hpp"

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024


/*设置为非阻塞*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式*/
void addfd(int epollfd, int fd, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
    {
        event.events |= EPOLLET; //条件满足，将其设为ET模式
    } 
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/*LT模式工作流程*/
void lt(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd) //如果是监听套接字证明有新的连接
        {
            struct sockaddr_in cli;
            socklen_t len = sizeof(cli);
            int connfd = accept(listenfd, (struct sockaddr*)&cli, &len);
            addfd(epollfd, connfd, false); //添加到epoll事件集中，LT模式
        }
        else if(events[i].events & EPOLLIN) 
        {
            /*只要socket读缓存中还有未读出的数据，这段代码就将触发*/
            printf("event trigger once\n");
            memset(buf, '\0', sizeof(buf));
            int ret = recv(sockfd, buf, BUFFER_SIZE -1, 0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n", ret, buf);
        }
        else
        {
            printf("something else happened \n");
        }
    }
}

/*ET工作模式流程*/
void et(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd) //有新的连接
        {
            struct sockaddr_in cli;
            socklen_t len = sizeof(cli);
            int connfd = accept(listenfd, (struct sockaddr*)&cli, &len);
            addfd(epollfd, connfd, true); //添加到epoll事件集中并且设置ET工作模式
        }
        else if(events[i].events & EPOLLIN)
        {
            /*因为是ET工作模式，所以这段代码不能被重复触发，所以我们循环读取数据，
            以确保把socket读缓存中的所有数据读出*/
            printf("event trigger once\n");
            while(1)
            {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    /*对于非阻塞I/O,下面的条件成立表示数据已经全部读取完毕。此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作*/
                    if( (errno = EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0)
                {
                    close(sockfd);   
                }
                else
                {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        }
        else 
        {
            printf("something else hanppend\n");
        }
    }
}