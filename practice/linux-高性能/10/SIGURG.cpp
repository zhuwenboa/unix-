#include"head.hpp"
#include<signal.h>

#define BUF_SIZE 1024
static int connfd;

/*SIGURG信号的处理函数*/
void sig_urg(int sig)
{
    int save_errno = errno;
    char buffer[BUF_SIZE] = {0};
    int ret = recv(connfd, buffer, BUF_SIZE-1, MSG_OOB); //接收带外数据
    printf("got %d bytes of oob data '%s' \n", ret, buffer);
    errno = save_errno;
}
/*
void addsig(int sig, void(*sig_handler)(int))
{   
    printf("----\n");
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART; 
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
*/
int main(int argc, char *argv[])
{
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int ret = 0;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
        my_err("socket", __LINE__);
    
    ret = bind(listenfd, (sockaddr *)&address, sizeof(address));
    if(ret < 0) 
        my_err("bind", __LINE__);
    
    ret = listen(listenfd, 5);
    if(ret < 0)
        my_err("listen", __LINE__);

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    connfd = accept(listenfd, (struct sockaddr*)&client, &len);
    if(connfd < 0)
        my_err("accpet", __LINE__);
    else
    {
        //addsig(SIGURG, sig_urg);
        /*使用SIGURG信号之前，我们必须设置socket的宿主进程或进程组*/
        //要接收SIGURG信号，我们的进程必须为套接口的所有者。
        fcntl(connfd, F_SETOWN, getpid());
        sig_urg(SIGURG);
        //sleep(1);
        char buffer[BUF_SIZE];
        while(1)/*循环接收普通数据*/
        {   
            memset(buffer, '\0', BUF_SIZE);
            ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
            if(ret <= 0)
                break;
            printf("got %d bytes of normal data '%s'\n", ret, buffer);

        }
        close(connfd);
    }

    close(listenfd);
    return 0;
    
}

