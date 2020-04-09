#include<iostream>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/timerfd.h>
#include<assert.h>
#include<time.h>

int main(int argc, char* argv[])
{
    struct itimerspec new_value;
    struct timespec now;
    uint64_t exp;
    ssize_t s;

    int ret = clock_gettime(CLOCK_REALTIME, &now);
    assert(ret != -1);

    //第一次到期的时间
    new_value.it_value.tv_sec = 5;
    new_value.it_value.tv_nsec = now.tv_nsec;

    //之后每次到期的时间间隔
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

    int timefd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(timefd != -1);

    ret = timerfd_settime(timefd, 0, &new_value, NULL); //启动定时器
    assert(ret != -1);

    int* a = &timefd;
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event events[126];
    struct epoll_event event;
    event.data.fd = timefd;
    event.data.ptr = a;
    event.events = EPOLLIN | EPOLLET;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, timefd, &event);
    while(true)
    {
        int num = epoll_wait(epollfd, events, 126, 0);
        assert(num >= 0);

        for(int i = 0; i < num; ++i)
        {
            if(events[i].events & EPOLLIN)
            {
                int* fd = static_cast<int*>(events[i].data.ptr);
                std::cout << *fd << "\n";
                if(*fd == timefd)
                {
                    s = read(*fd, &exp, sizeof(uint64_t));
                    assert(s == sizeof(exp));
                    std::cout << "time expired\n";
                }
            }
        }
    }
    close(timefd);
    close(epollfd);
    return 0;
}