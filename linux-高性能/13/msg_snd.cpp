#include<sys/msg.h>
#include<iostream>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
static int pipefd[2]; 

typedef struct 
{
    long type;
    char name[20];
    int age;
}MSG;

class msg
{
public:  
    msg();
    bool snd(); //向消息队列中加入消息
    bool rcv(); //从消息队列中取出消息

private: 
    key_t key;  //消息队列键值
    int msgid; //消息队列标识符
    MSG m;   //要发送的消息
    MSG r; //接收的消息
};

msg::msg()
{
     key_t key;
     //printf("key = : %d\n", key);
     msgid = msgget(key, IPC_CREAT | O_WRONLY | 0666);
     bzero(&m, 0);
     bzero(&r, 0);
     if(msgid < 0)
     {
         perror("msgget error!");
         exit(-1);
     }
     printf("请输入姓名: ");
     std::cin >> m.name;
     printf("请输入年龄： ");
     std::cin >> m.age;
     printf("输入信息的类型: ");
     std::cin >> m.type;
}
bool msg::snd()
{
    msgsnd(msgid, &m, sizeof(m) - sizeof(m.type), 0);
}

bool msg::rcv()
{
    msgrcv(msgid, &r, sizeof(r) - sizeof(r.type), 2, 0);
    printf("recv name: %s, age: %d\n", r.name, r.age);
}

int main()
{
    msg example;
    pipe(pipefd);
    pid_t pid = fork();
    if(pid < 0)
    {
        perror("fork is failure");
        exit(-1);
    }
    else if(pid == 0)
    {
        close(pipefd[0]);
        example.snd();
        char flag = '1';
        write(pipefd[1], &flag, 1);
    }
    else
    {
        char flag;
        //阻塞等待子进程通知
        read(pipefd[0], &flag, 1);
        example.rcv();        
    }
    int status;
    wait(&status);
    return 0;
}