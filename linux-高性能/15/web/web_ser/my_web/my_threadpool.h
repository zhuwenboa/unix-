#ifndef MY_THREADPOOL_H
#define MY_THREADPOOL_H
#include<list>
#include<pthread.h>
//#include"locker.h"
#include<iostream>
template<typename T> 
class my_threadpool
{
public:  
    my_threadpool(int thread_num = 6, int max_requests = 10000);
    ~my_threadpool();

    //向请求队列中添加任务
    bool append(T *request);

    //工作线程运行函数
    static void *worker(void *arg);
    void run();

private:  
    int my_thread_number;//线程中的线程数
    int my_max_requests; //请求队列中允许的最大请求数
    pthread_t *my_threads; //线程池数组
    std::list<T*>my_workqueue; //任务队列
    pthread_mutex_t my_queuelock; //保护锁
    pthread_cond_t my_queuecond; //条件变量
    bool m_stop; //是否结束线程
};

template<typename T>
my_threadpool<T>::my_threadpool(int thread_num, int max_requests)
{
    my_thread_number = thread_num;
    my_max_requests = max_requests;
    m_stop = false;
    pthread_mutex_init(&my_queuelock, NULL);
    pthread_cond_init(&my_queuecond, NULL);
    my_threads = new pthread_t[my_thread_number];
    if(!my_threads)
    {
        perror("创建线程池错误\n");
        exit(0);
    }
    for(int i = 0; i < my_thread_number; ++i)
    {
        std::cout << "create the " << i << "th thread\n";
        int ret = pthread_create(&my_threads[i], NULL, worker, this); //函数参数必须为静态的或者全局的函数
        if(ret != 0)
        {
            delete [] my_threads;
            perror("pthread_create false\n");
            exit(0);
        }
        ret = pthread_detach(my_threads[i]); //线程分离，不可被其它线程获pthread_join;
        //错误处理
    }
}

template<typename T>
my_threadpool<T>::~my_threadpool()
{
    delete [] my_threads;
    m_stop = true;
    pthread_mutex_destroy(&my_queuelock);
    pthread_cond_destroy(&my_queuecond);
}

template<typename T>
bool my_threadpool<T>::append(T *request)
{
    pthread_mutex_lock(&my_queuelock);
    if(my_workqueue.size() > my_max_requests)
    {
        pthread_mutex_unlock(&my_queuelock);
        return false;
    }
    my_workqueue.emplace_back(request);
    printf("workqueue size = %d\n", my_workqueue.size());
    pthread_mutex_unlock(&my_queuelock);
    pthread_cond_signal(&my_queuecond);
}

template<typename T> 
void* my_threadpool<T>::worker(void *arg)
{
    my_threadpool *pool = (my_threadpool*)arg;
    std::cout << "sizeof my_threadpool = " << sizeof(my_threadpool) << "\n";
    pool->run();
    return pool;
}

template<typename T>
void my_threadpool<T>::run()
{
    while(!m_stop)
    {
        /*这一步避免在所有线程去执行任务，然后这个时候有任务加入，signal之后没有线程去取任务，
        导致会有任务饿死，这一步也可以让线程更加高效的去执行任务。
        */
        //任务队列中有任务,直接去执行任务不等待唤醒
        pthread_mutex_lock(&my_queuelock);
        while(my_workqueue.empty())
        {
            pthread_cond_wait(&my_queuecond, &my_queuelock);
        }
        T *quest = my_workqueue.front();
        my_workqueue.pop_front();
        pthread_mutex_unlock(&my_queuelock);
        if(!quest)
            continue;
        quest->process();
    }
        
        /*
        if(!my_workqueue.empty())
        {
            pthread_mutex_lock(&my_queuelock);
            //如果争抢到锁，在进行一次判断，避免在拿到锁的这段时间任务被别的线程取走，导致任务队列为空
            if(my_workqueue.empty())
            {
                //如果队列为空释放锁
                pthread_mutex_unlock(&my_queuelock);
                continue;
            }
            T *quest = my_workqueue.front();
            my_workqueue.pop_front();
            pthread_mutex_unlock(&my_queuelock);
            if(!quest)
                continue;
            quest->process(); //执行回调函数
        }
        else
        {
            pthread_mutex_lock(&my_queuelock);
            int ret = pthread_cond_wait(&my_queuecond, &my_queuelock);
            if(ret == 0 && (!my_workqueue.empty()))
            {
                T *quest = my_workqueue.front();
                my_workqueue.pop_front();
                pthread_mutex_unlock(&my_queuelock);
                quest->process();
            }
        }
        */
}

#endif