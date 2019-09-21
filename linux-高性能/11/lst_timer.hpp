#ifndef LST_TIMER
#define LST_TIMER
#include"head.hpp"
#include<time.h>

#define BUFFER_SIZE 64
class util_timer;  

/*用户数据结构： 客户端socket地址 文件描述符 读缓存和定时器*/
struct client_data
{
    struct sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};
/*定时器类*/
class util_timer
{
public: 
    util_timer() : prev(NULL), next(NULL) {}

public: 
    time_t expier; //任务的超时时间，这里使用绝对时间
    void (*cb_func)(client_data*); //任务回调函数
    client_data *user_data;
    util_timer* prev; //指向前一个定时器
    util_timer* next; //指向后一个定时器

};

/*定时器链表。 它是一个升序，双向链表且带有头结点和尾节点*/
class sort_timer_lst
{
public: 
    sort_timer_lst() : head(NULL), tail(NULL) {}
    /*链表被销毁时，删除其中所有的定时器*/
    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }
    /*将目标定时器timer添加到链表中*/
    void add_timer(util_timer* timer);

    /*当某个定时器的任务发生变化时，调整对应的定时器在链表中的位置。*/
    void adjust_timer(util_timer* timer);

    /*将目标定时器timer从链表中移除*/
    void del_timer(util_timer* timer);

    /*SIGALRM信号每次被触发就在其信号处理函数（如果使用统一事件源，则是主函数）中执行一次tick函数，以处理链表上到期的任务*/
    void tick();

    /*重载的add_timer函数，它被公有的add_timer函数和adjust_timer的函数调用。
    该函数表示将目标定时器timer添加到节点lst_head之后的节点lst_head之后的部分链表中*/
    void add_timer(util_timer* timer, util_timer* lst_head);


public: 
    util_timer* head;
    util_timer* tail;
};


/*将目标定时器timer添加到链表中*/
void sort_timer_lst::add_timer(util_timer* timer)
{
    if(!timer)
    {
        return;
    }
    if(!head) //如果链表中无节点
    {
        head = tail = timer;
        return;
    }
    /*如果目标定时器的超时时间小于当前链表中所有定时器的超时时间，则把该节点插入到链表头节点。
     否则就需要调用重载函数add_timer(util_timer* timer, util_timer* lst_head), 把它插入链表中合适的位置，以保证链表的升序特性*/

    if(timer->expier < head->expier)
    {
        timer->next = head;
        head = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);
}

/*当某个定时器的任务发生变化时，调整对应的定时器在链表中的位置。*/
void sort_timer_lst::adjust_timer(util_timer* timer)
{
    if(!timer)
        return;
    /*当需要调整时，我们只需将此节点从链表中取出，然后重新插入*/
  //  util_timer* prev = timer->prev;
  //  util_timer* next = timer->next;
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    timer->prev = timer->next = NULL;
    add_timer(timer);
}


/*将目标定时器timer从链表中移除*/
void sort_timer_lst::del_timer(util_timer* timer)
{
    if(!timer)
        return;
    /*下面这个条件成立表示链表只有一个定时器，既目标定时器*/
    if( (timer == head) && (timer == tail) )
    {
        delete timer;
        head = tail = nullptr;
        return;
    }
    /*如果目标至少存在两个定时器，且目标定时器是链表的头结点，则将链表的头结点重置为原头结点的下一个节点，然后删除目标定时器*/
    if(timer == head)
    {
        head = timer->next;
        head->prev = nullptr;
        delete timer;
        return;
    }
    /*如果目标至少存在两个定时器，且目标定时器是链表的尾结点，则将链表的尾结点重置为原尾结点的上一个节点，然后删除目标定时器*/
    if(timer == tail)
    {
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
        return;
    }
    /*如果目标定时器位于链表的中间，则把它前后的定时器串联起来，然后删除目标定时器*/
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
    return;
}

/*SIGALRM信号每次被触发就在其信号处理函数（如果使用统一事件源，则是主函数）中执行一次tick函数，以处理链表上到期的任务*/
void sort_timer_lst::tick()
{
    if(!head)
        return;
    printf("timer tick\n");
    time_t cur = time(NULL); //获得系统的当前时间
    util_timer* tmp = head;
    /*从头结点开始依次处理每个定时器，直到遇到一个尚未到期的定时器，这就是定时器的核心逻辑*/
    while(tmp)
    {
        /*因为每个定时器都使用绝对时间作为超时值，所以我们可以把定时器的超时值和系统当前时间比较以判断定时器是否到期*/
        if(cur < tmp->expier)
            return; 
        /*调用定时器的回调函数，以执行定时任务*/
        tmp->cb_func(tmp->user_data);
        /*执行完定时器中的定时任务之后，就将它从链表中删除，并重置头结点*/
        head = tmp->next;

        if(head)
            head->prev = nullptr;
        delete tmp;
        tmp = head;
    }
}

/*重载的add_time函数*/
void sort_timer_lst::add_timer(util_timer* timer, util_timer* lst_head)
{
    util_timer* prev = lst_head;
    util_timer* tmp = lst_head->next;

    /*遍历lst_head节点之后的部分链表，直到找到一个超时时间大于目标定时器的超时时间的节点，并将目标定时器插入到该节点之前*/
    while(tmp)
    {
        if(timer->expier < tmp->expier)
        {
            timer->next =tmp;
            prev->next = timer;
            timer->prev = prev;
            tmp->prev = timer;
            break;
        }
        prev =tmp;
        tmp = tmp->next;
    }
    /*如果遍历完都没有插入，则将其插入到尾节点*/
    tmp = timer;
    prev->next = tmp;
    tmp->prev = prev;
    tmp->next = nullptr;
    tail = tmp;
}
#endif   