#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<iostream>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<sys/wait.h>
#include<memory>
#include<sys/types.h>
static const char* shm_name = "/my_shm"; //共享内存文件名
void pv(int sem_id, int op); //用信号量控制进程间同步

class share_mem
{
public: 
    share_mem();
    ~share_mem()
    {
        shm_unlink(shm_name);
    }
    bool write_mem(char* str); //向共享内存中写入数据
    bool read_mem(int len);           //从共享内存中读取数据


public:   
    int pipefd[2];   //通知主/子进程有数据可读/可写
private:  
    char write_buf[1024];
    char read_buf[1024];
    int shmfd;      //共享内存描述符
    std::shared_ptr<char*> sha_me; //指向共享内存的首地址
    
    //信号量
    int sem_id;
    union semun
    {
        int val;
        struct semid_ds* buf;
        unsigned short int* array;
        struct seminfo* _buf;
    }sem_un;     
};

share_mem::share_mem()
{
    pipe(pipefd); //创建管道
    bzero(write_buf, sizeof(write_buf));
    bzero(read_buf, sizeof(read_buf));
    //创建共享内存
    shmfd = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL, 0666);
    //改变共享内存大小
    ftruncate(shmfd, 1024);

    char* temp = (char*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    sha_me = std::make_shared<char*>(temp);
    //创建信号量
    sem_id = semget(IPC_PRIVATE, 1, 0666);
    sem_un.val = 1; //将信号量设置为1

    semctl(sem_id, 0, SETVAL, sem_un); //将信号量的semval值设置为1
}


//当op为-1时执行获取信号量操作，-1为释放信号量操作
void pv(int sem_id, int op)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0; 
    sem_b.sem_op = op;
    sem_b.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_b, 1);
}

//向共享内存写入数据
bool share_mem::write_mem(char* str)
{
    //像共享内存中写入数据，保证进程同步，op为-1 想要获取信号量
    pv(sem_id, -1);
    if(strlen(str) > 1023)
    {
        std::cout << "data is too long, the share_mem can't recv this data\n";
        return false;
    }
    else
    {
        for(int i = 0; i <= strlen(str); ++i)
        {
            (*sha_me)[i] = str[i];
        }
        std::cout << *sha_me << std::endl;
    }
    //将信号量加1，解除对信号量控制
    pv(sem_id, 1);
}

//读取数据
bool share_mem::read_mem(int len)
{
    pv(sem_id, -1);
    for(int i = 0; i <= len; ++i)
        read_buf[i] =(*sha_me)[i]; 
    std::cout << "I read buf:  " <<  read_buf << std::endl;
    pv(sem_id, 1);
}

int main()
{
    share_mem sha;
    pid_t pid = fork();
    if(pid < 0)
    {
        std::cout << "pid is false\n";
    }
    else if(pid == 0)
    {
        close(sha.pipefd[0]);
        char* str = "hello word";
        sha.write_mem(str);
        int flag = strlen(str);
        //通知主进程，共享内存中的数据写入，你可以读取
        write(sha.pipefd[1], (char*)&flag, 4);
    }
    else
    {
        close(sha.pipefd[1]);
        int flag;
        //如果共享内存中没有数据，就将阻塞住
        read(sha.pipefd[0], (char*)&flag, 4);
        sha.read_mem(flag);
    }
    int status;
    wait(&status);
    return 0;

}