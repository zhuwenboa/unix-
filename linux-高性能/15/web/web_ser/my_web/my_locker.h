#ifndef MY_LOCKER_H
#define MY_LOCKER_H
#include<iostream>
#include<pthread.h>

class my_locker
{
public:  
    //friend class my_cond; //将条件变量类声明为友元
    my_locker()
    {
        pthread_mutex_init(&my_mutex, NULL);
    }

private:  
    pthread_mutex_t my_mutex;

};

class my_cond
{
public:  
    friend class my_locker;
    my_cond()
    {
        pthread_cond_init(&cond, NULL);
    }    
    ~my_cond()
    {

    }
    bool wait()
    {
        
    }
private:  
    pthread_cond_t cond;
};


#endif