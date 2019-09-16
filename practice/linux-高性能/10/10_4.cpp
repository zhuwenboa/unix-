#include"head.hpp"
#include<signal.h>
#include<errno.h>
#define MAX_EVENT_NUMBER 1024
static int pipefd[2];

/*设置为非阻塞*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式*/
void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/*信号处理函数*/
void sig_handler(int sig)
{
    /*保留原来的errno，在函数最后恢复，以保证函数的可重入性*/
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0); //将信号值写入管道中，以通知主循环
    errno = save_errno;
}

/*设置信号的处理函数*/
void addsig(int sig)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART; //SA_RESTART标志以自动重启被该信号中断的系统调用
    sigfillset(&sa.sa_mask);      //int sigfillset(sigset_t* set) 在信号集中设置所有信号
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char *argv[])
{
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int ret = 0;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
        my_err("socket", __LINE__);
    
    ret = bind(listenfd, (sockaddr *)&address, sizeof(address));
    if(ret < 0) 
        my_err("bind", __LINE__);
    
    ret = listen(listenfd, 5);
    if(ret < 0)
        my_err("listen", __LINE__);
    
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    
    addfd(epollfd, listenfd);

    /*使用socketpair创建管道，注册pipefd[0]上的可读事件*/
    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
    if(ret < 0) 
        my_err("socketpair", __LINE__);
    
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    /*设置一些信号的处理函数*/
    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);

    bool stop_server = false;

    while(!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if( (number < 0) && (errno != EINTR) )
        {
            std::cout << "epoll failure\n";
            break;
        }
        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                /*如果就绪的文件描述符是listenfd，则处理新的连接*/
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int connfd = accept(listenfd, (sockaddr*)&client, &len);
                addfd(epollfd, connfd);
            }
            else if( (sockfd == pipefd[0]) && (events[i].data.fd & EPOLLIN) )
            {   
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if(ret == -1)
                    continue;
                else if(ret == 0)
                    continue;
                else
                {
                    /*因为每个信号值占一个字节，所以按字节来逐个接收信号。我们以SIGTERM为例，来说明如何安全的终止服务器主循环*/
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {   
                            case SIGCHLD: 
                            case SIGHUP: 
                                continue;
                            case SIGTERM: 
                            case SIGINT: 
                              stop_server = true;
                        }
                    }
                }
                
            }
        }
    }
    std::cout << "close fds\n";
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    return 0;
}