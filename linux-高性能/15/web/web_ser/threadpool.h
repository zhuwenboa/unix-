//半同步半反应堆模式
//该模式由主线程处理读写请求，将封装好的任务类添加到任务队列中，然后由工作线程取出任务进行处理
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>
#include"locker.h"

//线程池类,将它定义为模板类是为了代码复用。模板参数T是任务类
template<typename T>
class threadpool
{
public:  
    /*参数thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理请求的数量 */
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    //往请求队列中添加任务
    bool append(T* request);

    //工作线程运行函数
    static void* worker(void* arg);
    void run();

private:  
    int m_thread_number; //线程池中的线程数
    int m_max_requests; //请求队列中允许的最大请求数
    pthread_t* m_threads; //描述线程池的数组，其大小为m_thread_number
    std::list<T*>m_workqueue; //请求队列
    locker m_queuelocker; //保护请求队列的互斥锁
    sem m_queuestat; //是否有任务需要处理
    bool m_stop; //是否结束线程
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
        : m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL)
{
    if((thread_number <= 0) || (max_requests <= 0) )
        throw std::exception();
    //创建线程池
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads)
    {
        throw std::exception();
    }
    //创建thread_number个线程，并将它们设置为脱离线程
    for(int i = 0; i < m_thread_number; ++i)
    {
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete [] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i]))
        {
            delete [] m_threads;
            throw std::exception();
        }
    }            
}
            
template<typename T>  
threadpool<T>::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

//将任务添加到任务队列中
template<typename T>  
bool threadpool<T>::append(T* request)
{
    //操作工作队列是一定要加锁，因为它被所有线程共享
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    std::cout << "任务加入成功\n";
    std::cout << "任务队列中任务数目： " <<m_workqueue.size() << "\n";
    m_queuelocker.unlock();
    
    //将信号量加１，唤醒等待线程
    m_queuestat.post();
    return true;
}

template<typename T> 
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template<typename T>  
void threadpool<T>::run()
{
    while(!m_stop)
    {
        //信号量减１，阻塞等待被唤醒
        m_queuestat.wait();
        m_queuelocker.lock();
        //如果任务队列中无任务
        if(m_workqueue.empty())
        {
            //解锁
            m_queuelocker.unlock();
            continue;
        }
        std::cout << "取出任务\n";
        //将任务队列头取出
        T* request = m_workqueue.front();
        //将取出的任务从队列中删掉
        m_workqueue.pop_front();
        //-----------------------
        m_queuelocker.unlock();
        if(!request)
        {
            continue;
        }
        pthread_t tid = pthread_self(); 
        std::cout << tid << "执行回调函数\n";
        //--------------------
        //m_queuelocker.unlock();
        request->process(); //执行回调函数
    }
}
#endif