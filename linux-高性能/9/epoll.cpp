#include<iostream>
#include"head.hpp"
#include<memory>

class Epoll
{
public:
    /*构造函数*/
    Epoll(int f) : fd(f), lt_event(std::make_shared<epoll_event>()), et_event(std::make_shared<epoll_event>()) {}
    Epoll(int f, epoll_event* a, epoll_event* b) :fd(f), lt_event(std::make_shared<epoll_event>(a)),
                                            et_event(std::make_shared<epoll_event>(b)) {}
    /*
    Epoll(epoll_event* a) : lt_event(std::make_shared<epoll_event>(a)),
                            et_event(std::make_shared<epoll_event>()) {}
    */

    /*将文件描述符设置成非阻塞的*/
    void setnonblocking();

    /*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式*/
    void addfd(bool);

    /*LT模式的工作流程*/
    void lt(epoll_event*, int, int, int);

    /*ET模式工作流程*/
    void et(epoll_event*, int, int, int);

private: 
    int fd;
    std::shared_ptr<epoll_event> lt_event;
    std::shared_ptr<epoll_event> et_event;
};

void Epoll::setnonblocking()
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK; 
    fcntl(fd, F_SETFL, new_option);
}

void Epoll::addfd(bool enabled_et)
{
                  
}