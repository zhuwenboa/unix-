#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/sem.h>

union semun
{
    int val;
    struct semid_ds* buf;
    unsigned short int* array;
    struct  seminfo* _buf;    
};

//op为-1时执行p操作，op为1时执行v操作
void pv(int sem_id, int op)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = op;    
    sem_b.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_b, 1);
}

int main(int argc, char* argv[])
{
    int sem_id = semget(IPC_PRIVATE, 1, 0666);

    union semun sem_un;
    sem_un.val = 1;
    semctl(sem_id, 0, SETVAL, sem_un);

    pid_t pid = fork();
    if(pid < 0)
        return -1;
    else if(pid == 0)    //子进程运行
    {
        std::cout << "chlid try to get binary sem \n";
        /*在父、子进程间共享IPC_PRIVATE信号量的关键就在于二者都可以操作该信号量的标识符sem_id*/
        pv(sem_id, -1);
        std::cout << "child get the sem and would release it after 5 seconds \n";
        sleep(5);
        pv(sem_id, 1);
        exit(0);
    }
    else
    {
        std::cout << "parent try to get binary sem\n";
        pv(sem_id, -1);
        sleep(5);
        std::cout << "parent get the sem and would release it after 5 seconds\n";
        pv(sem_id, 1);
        std::cout << "parent will sleep 3 seconds\n";
        sleep(3);
        std::cout << "parent weak and will exit\n";
    }

    waitpid(pid, NULL, 0);  //回收子进程结束的资源 避免僵尸进程的产生
    semctl(sem_id, 0, IPC_RMID, sem_un); //删除信号量
    return 0;

}