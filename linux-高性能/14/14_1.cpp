//死锁情况
#include<pthread.h>
#include<iostream>
#include<unistd.h>

int a = 0;
int b = 0;
pthread_mutex_t mutex_a;
pthread_mutex_t mutex_b;

void* anthor(void* arg)
{
    pthread_mutex_lock(&mutex_b);
    std::cout << "in child thread, got mutex b, waiting for mutex a\n";
    sleep(5);
    ++b;
    pthread_mutex_lock(&mutex_a);
    b += a++;
    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);
    pthread_exit(NULL);
}

int main()
{
    pthread_t id;
    //初始化锁
    pthread_mutex_init(&mutex_a, NULL);
    pthread_mutex_init(&mutex_b, NULL);
    //创建线程
    pthread_create(&id, NULL, anthor, NULL);
    
    pthread_mutex_lock(&mutex_a);
    std::cout << "in parent thread, got mutex a, waiting for mutex b\n";
    sleep(5);
    ++a;
    pthread_mutex_lock(&mutex_b);
    a += b++;
    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);

    //等待线程id结束
    pthread_join(id, NULL);
    //销毁锁
    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);
}